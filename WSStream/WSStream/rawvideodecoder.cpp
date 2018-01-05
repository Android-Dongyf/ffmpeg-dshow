#include "rawvideodecoder.h"

#include "libswscale/swscale.h"
RawVideoDecoder::RawVideoDecoder()
:decoder()
{

}
RawVideoDecoder::~RawVideoDecoder(){
    closeDecoder();
}

bool RawVideoDecoder::init(AVFormatContext *fmtCtx){
    bool ret = openDecoder(fmtCtx);
    if(!ret) {
       qDebug() << "openDecoder fail.";
    }

    mCvtCtx = sws_getContext(mCodeCtx->width, mCodeCtx->height, mCodeCtx->pix_fmt, mCodeCtx->width,
                             mCodeCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    mFrame422 = av_frame_alloc();
    //mFrame420 = av_frame_alloc();
    //unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, mCodeCtx->width, mCodeCtx->height));
    //avpicture_fill((AVPicture *)mFrame420, out_buffer, AV_PIX_FMT_YUV420P, mCodeCtx->width, mCodeCtx->height);
    /*mFrame420->format = AV_PIX_FMT_YUV420P;
    mFrame420->width  = mCodeCtx->width;
    mFrame420->height = mCodeCtx->height;
    int rc = av_frame_get_buffer(mFrame420, 32);
    if(rc < 0){
        ret = false;
    }*/

    return ret;
}

bool RawVideoDecoder::decode(AVPacket *pkt, AVFrame *frame){
    int ret, gotFrame = 0, rc;
    static int64_t lastPts = 0;
    //qDebug() << "RawVideoDecoder: " << pkt->stream_index;
    ret = avcodec_decode_video2(mCodeCtx, mFrame422, &gotFrame, pkt);
    if(ret < 0){
        qDebug() << "RawVideoDecoder decode errer";
        return false;
    }
    //qDebug() << "ret: " << ret << " gotFrame: " << gotFrame;
    if(gotFrame){
        if(lastPts != mFrame422->pts){
             frame->pts = mFrame422->pts;
             lastPts = mFrame422->pts;
        }
        else{
            qDebug() << "RawVideoDecoder ret: " << ret;
            qDebug() << "RawVideoDecoder   pts is same";
            frame->pts = mFrame422->pts;
            lastPts = mFrame422->pts;
            return false;
        }

       sws_scale(mCvtCtx, (const unsigned char* const*)mFrame422->data, mFrame422->linesize, 0, mCodeCtx->height, frame->data, frame->linesize);
       frame->width = mCodeCtx->width;
       frame->height = mCodeCtx->height;
       frame->format = AV_PIX_FMT_YUV420P;
#if Debug
       printf("lastPts: %lld pts: %lld\n", lastPts, mFrame422->pts);
#endif
    }
    if(gotFrame)
        return true;
    else {
        qDebug() << "gotFrame: " << gotFrame;
        return false;
    }

    if(pkt == NULL || frame == NULL) {
        printf("pkt == NULL or frame == NULL\n");
        return false;
    }

    ret = avcodec_send_packet(mCodeCtx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(mCodeCtx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            printf("avcodec_receive_frame ret: %d\n", ret);
            return false;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return false;
        }
#if Debug
        printf("dec frame_number %3d\n", mCodeCtx->frame_number);
#endif
    }

    return true;
}

bool RawVideoDecoder::openDecoder(AVFormatContext *fmtCtx){
    if(fmtCtx == NULL)
        return false;

    int videoIndex = -1;
    for(unsigned int i = 0; i < fmtCtx->nb_streams; i++)
        if(fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
               videoIndex = i;
               break;
        }
#if Debug
    printf("videoIndex: %d\n", videoIndex);
#endif
    mCodeCtx = fmtCtx->streams[videoIndex]->codec;
    mCodec = avcodec_find_decoder(mCodeCtx->codec_id);
    if (!mCodec) {
       fprintf(stderr, "Codec not found\n");
       return false;
    }

    /*mParser = av_parser_init(mCodec->id);
    if (!mParser) {
        fprintf(stderr, "parser not found\n");
        return false;
    }*/
/*
    mCodeCtx = avcodec_alloc_context3(mCodec);
    if(!mCodeCtx){
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }

    mCodeCtx->pix_fmt = AV_PIX_FMT_YUYV422;
    mCodeCtx->coded_width = 352;
    mCodeCtx->coded_height = 288;
    AVRational framerate = {25, 1};
    mCodeCtx->framerate = framerate;
*/
    /* open it */
    if (avcodec_open2(mCodeCtx, mCodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        //av_parser_close(mParser);
        return false;
    }

    return true;
}

bool RawVideoDecoder::closeDecoder(){
    if(mParser)
        av_parser_close(mParser);

    if(mFrame422)
        av_frame_free(&mFrame422);
    //av_frame_free(&mFrame420);
    return true;
}
