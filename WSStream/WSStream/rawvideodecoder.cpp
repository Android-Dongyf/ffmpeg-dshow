#include "rawvideodecoder.h"

#include "libswscale/swscale.h"
RawVideoDecoder::RawVideoDecoder()
:decoder()
{
    mCvtCtx = NULL;
    mFrame422 = NULL;
    videoCrop = NULL;
}
RawVideoDecoder::~RawVideoDecoder(){
    closeDecoder();

    if(videoCrop){
        videoCrop->deleteLater();
        videoCrop = NULL;
    }
}

bool RawVideoDecoder::init(AVFormatContext *fmtCtx){
    bool ret = openDecoder(fmtCtx);
    if(!ret) {
       qDebug() << "openDecoder fail.";
    }

    mCvtCtx = sws_getContext(StreamConfig::video_crop_width_val()/*mCodeCtx->width*/, StreamConfig::video_crop_height_val()/*mCodeCtx->height*/,
                             mCodeCtx->pix_fmt, StreamConfig::video_crop_width_val()/*mCodeCtx->width*/,
                             StreamConfig::video_crop_height_val()/*mCodeCtx->height*/, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    mFrame422 = av_frame_alloc();
    if(mFrame422 == NULL){
        return false;
    }
    mFrame420 = av_frame_alloc();
    if(mFrame420 == NULL){
        return false;
    }
    //unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, mCodeCtx->width, mCodeCtx->height));
    //avpicture_fill((AVPicture *)mFrame420, out_buffer, AV_PIX_FMT_YUV420P, mCodeCtx->width, mCodeCtx->height);
    /*mFrame420->format = AV_PIX_FMT_YUV420P;
    mFrame420->width  = mCodeCtx->width;
    mFrame420->height = mCodeCtx->height;
    int rc = av_frame_get_buffer(mFrame420, 32);
    if(rc < 0){
        ret = false;
    }*/

    videoCrop = new VideoCropFilter();
    if(videoCrop){
        ret = videoCrop->initWithInputFmtCtxAndDecCtx(fmtCtx, mCodeCtx);
        if(!ret){
            qDebug() << "initWithInputFmtCtxAndDecCtx fail.";
            return false;
        }
        qDebug() << "initWithInputFmtCtxAndDecCtx success.";
    }

    return ret;
}

bool RawVideoDecoder::decode(AVPacket *pkt, AVFrame *frame){
    int ret, gotFrame = 0;
    bool rc = true;

    ret = avcodec_decode_video2(mCodeCtx, mFrame422, &gotFrame, pkt);
    if(ret < 0){
        qDebug() << "RawVideoDecoder decode errer";
        return false;
    }
    qDebug() << "ret: " << ret << " gotFrame: " << gotFrame << " fmt: " << mFrame422->format;
    if(gotFrame){
       //²Ã¼ôÊÓÆµÊý¾Ý
       if(videoCrop){
           rc = videoCrop->cropVideoWithFrame(mFrame422, &mFrame420);
           if(!rc){
               qDebug() << "cropVideoWithFrame fail.";
               av_frame_unref(mFrame422);
               return false;
           }
           qDebug() << "cropVideoWithFrame success.";
       }

       sws_scale(mCvtCtx, (const unsigned char* const*)mFrame420->data, mFrame420->linesize, 0, StreamConfig::video_crop_height_val(), frame->data, frame->linesize);
       frame->width = StreamConfig::video_crop_width_val();
       frame->height = StreamConfig::video_crop_height_val();
       frame->format = AV_PIX_FMT_YUV420P;
       frame->pts = mFrame422->pts;
       av_frame_unref(mFrame422);
       av_frame_unref(mFrame420);
       return true;
    }
    else {
        av_frame_unref(mFrame422);
        av_frame_unref(mFrame420);
        return false;
    }
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

    if(mCvtCtx){
        sws_freeContext(mCvtCtx);
    }

    if(mFrame422){
        av_frame_free(&mFrame422);
        mFrame422 = NULL;
    }
    //av_frame_free(&mFrame420);

    if(mCodeCtx){
        avcodec_free_context(&mCodeCtx);
    }

    return true;
}
