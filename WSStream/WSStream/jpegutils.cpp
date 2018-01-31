#include "jpegutils.h"

JpegUtils::JpegUtils()
:OutputStream()
{

}

JpegUtils::~JpegUtils(){
    qDebug() << "JpegUtils::~JpegUtils";
    closeStream();
}

bool JpegUtils::init(bool isMaster){
    bool ret =true;

    mEncoder = new JpegEncoder();
    if(mEncoder) {
        ret = mEncoder->init();
        if(!ret) {
            qDebug() << "encoder fail. ";
            return false;
        }
    }

    ret = openStream();
    if(!ret) {
        qDebug() << "openStream fail. ";
        return false;
    }


    return true;
}

bool JpegUtils::openStream(){
    /* allocate the output media context */
    avformat_alloc_output_context2(&mOutFmtCtx, NULL, StreamConfig::jpeg_encoder_fmt_val().c_str()/*DEFAULT_OUTPUT_FMT*/, StreamConfig::jpeg_file_name_val().c_str()/*DEFAULT_OUTPUT_STREAM*/);
    if (!mOutFmtCtx) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&mOutFmtCtx, NULL, "mjpeg", StreamConfig::jpeg_file_name_val().c_str());
    }
    if (!mOutFmtCtx) {
        return false;
    }

    mOutFmt = mOutFmtCtx->oformat;

    mOutStream = avformat_new_stream(mOutFmtCtx, NULL);
    if (!mOutStream) {
        qDebug() << "Could not allocate stream. ";
        avformat_free_context(mOutFmtCtx);
        return false;
    }

    mOutStream->id = mOutFmtCtx->nb_streams - 1;

    if(mEncoder) {
        AVCodecContext *c = mEncoder->getCodecContex();
        /* copy the stream parameters to the muxer */
        int ret = avcodec_parameters_from_context(mOutStream->codecpar, c);
        if (ret < 0) {
            qDebug() << "Could not copy the stream parameters. ";
            avformat_free_context(mOutFmtCtx);
            return false;
        }
    }

    //av_dump_format(mOutFmtCtx, 0, StreamConfig::jpeg_file_name_val().c_str(), 1);

    /* open the output file, if needed */
    if (!(mOutFmt->flags & AVFMT_NOFILE)) {
        AVDictionary *opt = NULL;
        int ret = avio_open2(&mOutFmtCtx->pb, StreamConfig::jpeg_file_name_val().c_str(), AVIO_FLAG_WRITE, NULL, &opt);
        if (ret < 0) {
            avformat_free_context(mOutFmtCtx);
            qDebug() << "avio_open2 fail.";
            return false;
        }
    }

    writeHeaderToStream();
}

bool JpegUtils::closeStream(){
    if(mEncoder)
        delete mEncoder;

    if (!(mOutFmt->flags & AVFMT_NOFILE))
            /* Close the output file. */
            avio_closep(&mOutFmtCtx->pb);

    if(mOutFmtCtx) {
        avformat_free_context(mOutFmtCtx);
        mOutFmtCtx = NULL;
    }
}

bool JpegUtils::writeOneFrameToStream(AVFrame *frame, enum AVMediaType codec_type, int *err_code){
    AVPacket *pkt = NULL;
    bool ret = true;
    *err_code = 0;
    pkt = av_packet_alloc();
    if(!pkt) {
        qDebug() << "pkt ==NULL";
        *err_code = -1;
        return false;
    }
    // qDebug() << "writeOneFrameToStream 1111111111111111111111";
    if(codec_type == AVMEDIA_TYPE_VIDEO){
        bool ret = mEncoder->encode(frame, pkt);
        if(!ret){
            printf("encode fail.\n");
            av_packet_free(&pkt);
            pkt = NULL;
            *err_code = -1;
            return false;
        }
    }
    //qDebug() << "writeOneFrameToStream 22222222222222222222222";
    if(pkt){
        ret = writeFrameToStream(pkt, codec_type);
        if(!ret)
            *err_code = -2;
        av_packet_free(&pkt);
        pkt = NULL;
    }
   // qDebug() << "writeOneFrameToStream 333333333333333333333";
    return ret;
}

bool JpegUtils::writeHeaderToStream(){
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

bool JpegUtils::getOutStreamStatus(){
    return true;
}

encoder *JpegUtils::getAudioCodec(){
    return NULL;
}

bool JpegUtils::writeFrameToStream(AVPacket *pkt, enum AVMediaType codec_type){
    //qDebug() << "size: " << pkt->size;
    //qDebug() << "writeOneFrameToStream 44444444444444444444444";
    int ret = av_interleaved_write_frame(mOutFmtCtx, pkt);
    //qDebug() << "writeOneFrameToStream 555555555555555555555";
    //int ret = av_write_frame(mOutFmtCtx, pkt);
    if(ret < 0){
        qDebug() << "av_interleaved_write_frame fail.";
        return false;
    }

#if Debug
    qDebug() << "av_interleaved_write_frame success.";
#endif

    return true;
}
