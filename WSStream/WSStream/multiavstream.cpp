#include "multiavstream.h"

#include <QMutexLocker>

#include "mpeg1videoencoder.h"
#include "mp2audioencoder.h"
#include <QThread>

MultiAVStream::MultiAVStream()
:OutputStream()
{

}

MultiAVStream::~MultiAVStream() {
    if(mVideoEncoder)
        delete mVideoEncoder;

    if(mAudioEncoder)
        delete mAudioEncoder;
}

bool MultiAVStream::init() {
    mVideoEncoder = new Mpeg1VideoEncoder();
    if(!mVideoEncoder) {
        printf("Mpeg1VideoEncoder fail. \n");
        return false;
    }

    bool ret = mVideoEncoder->init();
    if(!ret) {
        printf("mVideoEncoder init fail.\n");
        return false;
    }

    mAudioEncoder = new Mp2AudioEncoder();
    if(!mAudioEncoder) {
        printf("Mp2AudioEncoder fail. \n");
        return false;
    }

    ret = mAudioEncoder->init();
    if(!ret) {
        printf("mAudioEncoder init fail.\n");
        return false;
    }

    ret = openStream();
    if(!ret) {
        printf("openStream fail.\n");
        return false;
    }

    return ret;
}

bool MultiAVStream::writeHeaderToStream(){
#if Debug
    printf("MultiAVStream::writeHeaderToStream\n");
#endif
    AVDictionary *opt = NULL;
    /* Write the stream header, if any. */
    int ret = avformat_write_header(mOutFmtCtx, &opt);
    if (ret < 0) {
        printf("avformat_write_header ret: %d\n", ret);
       // fprintf(stderr, "Error occurred when opening output file: %s\n",
         //       av_err2str(ret));
        return false;
    }

    return true;
}

/*static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}*/

bool MultiAVStream::writeFrameToStream(AVPacket *pkt, enum AVMediaType codec_type){
    /* rescale output packet timestamp values from codec to stream timebase */
    if(codec_type == AVMEDIA_TYPE_VIDEO) {
        //qDebug() << "framerate: " << atoi(StreamConfig::video_framerate_val().c_str());
        AVRational timeBase = {1, atoi(StreamConfig::video_framerate_val().c_str())/*DEFAULT_STREAM_FRAMERATE*/};
        av_packet_rescale_ts(pkt, timeBase, mOutStream->time_base);
        pkt->stream_index = mOutStream->index;
    }

    if(codec_type == AVMEDIA_TYPE_AUDIO) {
        AVRational timeBase = {1, StreamConfig::audio_sample_rate_val() };
        av_packet_rescale_ts(pkt, timeBase, mOutAudioStream->time_base);
        pkt->stream_index = mOutAudioStream->index;
    }


    mWriteLock.lock();
    if(!mOutFmtCtx) {
        printf("mOutFmtCtx == NULL \n");
        mWriteLock.unlock();
        return false;
    }

    int ret = av_interleaved_write_frame(mOutFmtCtx, pkt);
    if(ret < 0){
        qDebug() << "av_interleaved_write_frame fail.";
        mWriteLock.unlock();
        return false;
    }
#if 1
   qDebug() << "av_interleaved_write_frame success.codec_type: " << codec_type;
#endif
    mWriteLock.unlock();
    return true;
}

bool MultiAVStream::writeOneFrameToStream(AVFrame *frame, enum AVMediaType codec_type, int *err_code){
    AVPacket *pkt = NULL;
    bool ret = true;
    int start_time = av_gettime();
    *err_code = 0 ;
    pkt = av_packet_alloc();
    if(!pkt) {
        qDebug() << "pkt ==NULL";
        *err_code = -1;
        return false;
    }

    if(codec_type == AVMEDIA_TYPE_VIDEO){
        bool ret = mVideoEncoder->encode(frame, pkt);

        AVRational time_base={1, 10000000};
        AVRational time_base_q={1, AV_TIME_BASE};
        int64_t pts_time = av_rescale_q(pkt->dts, time_base, time_base_q);
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time)
            av_usleep(pts_time - now_time);

        if(!ret){
            qDebug() << "AVMEDIA_TYPE_VIDEO encode fail.";
            av_packet_free(&pkt);
            pkt = NULL;
            *err_code = -1;
            return false;
        }
        qDebug() << "MultiAVStream AVMEDIA_TYPE_VIDEO encode success.";
    }

    if(codec_type == AVMEDIA_TYPE_AUDIO) {
        static int64_t pts = 0;
        if(frame){
            frame->pts = pts++;
            pts += frame->nb_samples;
        }

        bool ret = mAudioEncoder->encode(frame, pkt);

        AVRational time_base = {1, 10000000};
        AVRational time_base_q = {1, AV_TIME_BASE};
        int64_t pts_time = av_rescale_q(pkt->dts, time_base, time_base_q);
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time)
            av_usleep(pts_time - now_time);

        if(!ret){
            qDebug() << "AVMEDIA_TYPE_AUDIO encode fail.";
            av_packet_free(&pkt);
            pkt = NULL;
            *err_code = -1;
            return false;
        }
        qDebug() << "MultiAVStream AVMEDIA_TYPE_AUDIO encode success.";
    }

    mStatusLock.lock();
    if(!mStatusOk){
        mStatusLock.unlock();
        av_packet_free(&pkt);
        pkt = NULL;
        return false;
    }
    if(pkt){
        ret = writeFrameToStream(pkt, codec_type);
        if(!ret)
            *err_code = -2;
        av_packet_free(&pkt);
        pkt = NULL;
    }
    mStatusLock.unlock();
    *err_code = 0;
    return ret;
}

bool MultiAVStream::openStream(){
    mStatusLock.lock();
    char url[128] = {0};
    sprintf(url, "http://%s:%d/%s/%s", StreamConfig::stream_output_addr_val().c_str(),
            StreamConfig::stream_output_port_val(), StreamConfig::stream_output_secret_val().c_str(), StreamConfig::stream_output_index_val().c_str());
    qDebug() << "url: " << url;
    /* allocate the output media context */
    avformat_alloc_output_context2(&mOutFmtCtx, NULL, StreamConfig::stream_output_fmt_val().c_str()/*DEFAULT_OUTPUT_FMT*/, url/*DEFAULT_OUTPUT_STREAM*/);
    if (!mOutFmtCtx) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&mOutFmtCtx, NULL, "mpegts", url/*DEFAULT_OUTPUT_STREAM*/);
    }
    if (!mOutFmtCtx) {
        mStatusLock.unlock();
        return false;
    }

    mOutFmt = mOutFmtCtx->oformat;
    if(mOutFmt->video_codec == AV_CODEC_ID_MPEG2VIDEO)
        mOutFmt->video_codec = AV_CODEC_ID_MPEG1VIDEO;
#if 1
    //printf("nb_streams: %d\n", mOutFmtCtx->nb_streams);
    mOutStream = avformat_new_stream(mOutFmtCtx, NULL);
    if (!mOutStream) {
        fprintf(stderr, "Could not allocate stream\n");
        avformat_free_context(mOutFmtCtx);
        mStatusLock.unlock();
        return false;
    }
    //printf("nb_streams: %d\n", mOutFmtCtx->nb_streams);
    mOutStream->id = mOutFmtCtx->nb_streams - 1;
    AVRational timeBase = {1, StreamConfig::stream_output_framerate_val()/*DEFAULT_STREAM_FRAMERATE*/};
    //AVRational timeBase = {1, 90000};
    mOutStream->time_base = timeBase;

    if(mVideoEncoder) {
        AVCodecContext *c = mVideoEncoder->getCodecContex();
        /* copy the stream parameters to the muxer */
        int ret = avcodec_parameters_from_context(mOutStream->codecpar, c);
        if (ret < 0) {
            fprintf(stderr, "Could not copy the stream parameters\n");
            avformat_free_context(mOutFmtCtx);
            mStatusLock.unlock();
            return false;
        }
    }
#endif
#if 1
    mOutAudioStream = avformat_new_stream(mOutFmtCtx, NULL);
    if (!mOutAudioStream) {
        fprintf(stderr, "Could not allocate stream\n");
        avformat_free_context(mOutFmtCtx);
        mStatusLock.unlock();
        return false;
    }

    mOutAudioStream->id = mOutFmtCtx->nb_streams - 1;
    AVRational timeBase1 = {1, 32000};
    //AVRational timeBase1 = {1, 90000};
    mOutAudioStream->time_base = timeBase1;

    if(mAudioEncoder) {
        AVCodecContext *c = mAudioEncoder->getCodecContex();
        /* copy the stream parameters to the muxer */
        int ret = avcodec_parameters_from_context(mOutAudioStream->codecpar, c);
        if (ret < 0) {
            fprintf(stderr, "Could not copy the stream parameters\n");
            avformat_free_context(mOutFmtCtx);
            mStatusLock.unlock();
            return false;
        }
    }
#endif
    av_dump_format(mOutFmtCtx, 0, url, 1);

    /* open the output file, if needed */
    if (!(mOutFmt->flags & AVFMT_NOFILE)) {
        AVDictionary *opt = NULL;
        //av_dict_set(&opt, "content_type", "multipart/form-data", 0);

        //av_dict_set_int(&opt, "multiple_requests", 1, 0);
        //av_dict_set_int(&opt, "chunked_post", 0, 0);
        //int ret = avio_open(&mOutFmtCtx->pb, DEFAULT_OUTPUT_STREAM, AVIO_FLAG_WRITE);
        int ret = avio_open2(&mOutFmtCtx->pb, url, AVIO_FLAG_WRITE, NULL, &opt);
        if (ret < 0) {
            avformat_free_context(mOutFmtCtx);
            //fprintf(stderr, "Could not open '%s': %s\n", filename,
            //        av_err2str(ret));
            char buf[1024] = {0};
            av_strerror(ret, buf, 1024);
            printf("avio_open %s fail. ret: %d %s\n", url, ret, buf);
            mStatusLock.unlock();
            return false;
        }
    }


    mStatusOk = true;
    writeHeaderToStream();
    mStatusLock.unlock();
    return true;
}

bool MultiAVStream::getOutStreamStatus(){
    mStatusLock.lock();
    bool ret = mStatusOk;
    mStatusLock.unlock();
    return ret;
}

bool MultiAVStream::closeStream(){
    mStatusLock.lock();
    if (!(mOutFmt->flags & AVFMT_NOFILE))
            /* Close the output file. */
            avio_closep(&mOutFmtCtx->pb);

    if(mOutFmtCtx) {
        avformat_free_context(mOutFmtCtx);
        mOutFmtCtx = NULL;
    }

    mStatusOk = false;
    mStatusLock.unlock();
    return true;
}

encoder *MultiAVStream::getAudioCodec() {
    return mAudioEncoder;
}
