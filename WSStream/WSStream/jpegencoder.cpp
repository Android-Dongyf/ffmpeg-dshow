#include "jpegencoder.h"
#include "streamconfig.h"

JpegEncoder::JpegEncoder()
:encoder()
{

}

JpegEncoder::~JpegEncoder(){
    qDebug() << "JpegEncoder::~JpegEncoder()";
    closeEncoder();
}

bool JpegEncoder::init(){
    bool ret = openEncoder();
    if(!ret) {
       qDebug() << "openEncoder fail. ";
    }

    return ret;
}

bool JpegEncoder::encode(AVFrame *frame, AVPacket *pkt){
    int ret, got_packet = 0;
    //qDebug() << "JpegEncoder::encode 111111111111";
    ret = avcodec_encode_video2(mCodecCtx, pkt, frame, &got_packet);
    //qDebug() << "JpegEncoder::encode 22222222222222";
    if(ret < 0) {
        qDebug() << "avcodec_encode_video2 fail.";
        return false;
    }
    if(got_packet) {
        //qDebug() << "avcodec_encode_video2 success.";
        return true;
    }
    else
        return false;
}

AVCodecContext *JpegEncoder::getCodecContex(){
    return mCodecCtx;
}

bool JpegEncoder::openEncoder(){
    const char *codec_name = DEFAULT_ENCODE_NAME;
    int ret;

    /* find the mpeg1video encoder */
    mCodec = avcodec_find_encoder_by_name(codec_name);
    if (!mCodec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        return false;
    }

    mCodecCtx = avcodec_alloc_context3(mCodec);
    if (!mCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }


    /* resolution must be a multiple of two */
    mCodecCtx->width = StreamConfig::video_encode_width_val();//DEFAULT_ENCODE_WIDTH;
    mCodecCtx->height = StreamConfig::video_encode_height_val();//DEFAULT_ENCODE_HEIGHT;
    AVRational timeBase = {1, 30};
    mCodecCtx->time_base = timeBase;
    mCodecCtx->pix_fmt = DEFAULT_ENCODE_FORMAT;

    ret = avcodec_open2(mCodecCtx, mCodec, NULL);
    if (ret < 0) {
        qDebug() << "avcodec_open2 error. ";
        avcodec_free_context(&mCodecCtx);
        return false;
    }

    return true;
}

bool JpegEncoder::closeEncoder(){
    if(mCodecCtx) {
        avcodec_free_context(&mCodecCtx);
        mCodecCtx = NULL;
    }
}
