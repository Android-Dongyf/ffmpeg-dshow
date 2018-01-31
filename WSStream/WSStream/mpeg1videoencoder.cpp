#include "mpeg1videoencoder.h"

Mpeg1VideoEncoder::Mpeg1VideoEncoder():encoder()
{

}

Mpeg1VideoEncoder::~Mpeg1VideoEncoder(){
    closeEncoder();
}

bool Mpeg1VideoEncoder::init(){
    bool ret = openEncoder();
    if(!ret) {
       printf("openEncoder fail. \n");
    }

    return ret;
}

bool Mpeg1VideoEncoder::encode(AVFrame *frame, AVPacket *pkt){
    int ret, got_packet = 0;

    ret = avcodec_encode_video2(mCodecCtx, pkt, frame, &got_packet);
    if(ret < 0) {
        qDebug() << "avcodec_encode_video2 fail.";
        return false;
    }
    if(got_packet){
        if (mCodecCtx->coded_frame->key_frame)
        {
            pkt->flags |= AV_PKT_FLAG_KEY;
        }
        return true;
    }
    else
        return false;
}

bool Mpeg1VideoEncoder::openEncoder(){
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

    mPkt = av_packet_alloc();
    if (!mPkt){
        avcodec_free_context(&mCodecCtx);
        return false;
    }
    if(mCodecCtx->codec_id = AV_CODEC_ID_MPEG1VIDEO)
        mCodecCtx->mb_decision = 2;
    /* put sample parameters */
    mCodecCtx->bit_rate = StreamConfig::video_encode_rate_val();//DEFAULT_ENCODE_BITRATE;
    mCodecCtx->rc_min_rate = StreamConfig::video_encode_rate_val();//DEFAULT_ENCODE_BITRATE;
    mCodecCtx->rc_max_rate = StreamConfig::video_encode_rate_val();//DEFAULT_ENCODE_BITRATE;
    mCodecCtx->bit_rate_tolerance = StreamConfig::video_encode_rate_val();//DEFAULT_ENCODE_BITRATE;
    mCodecCtx->rc_buffer_size = StreamConfig::video_encode_rate_val();//DEFAULT_ENCODE_BITRATE;
    mCodecCtx->rc_initial_buffer_occupancy = mCodecCtx->rc_buffer_size*3/4;
   // mCodecCtx->rc_buffer_aggressivity= (float)1.0;
    //mCodecCtx->rc_initial_cplx= 0.5;
    /* resolution must be a multiple of two */
    mCodecCtx->width = StreamConfig::video_encode_width_val();//DEFAULT_ENCODE_WIDTH;
    mCodecCtx->height = StreamConfig::video_encode_height_val();//DEFAULT_ENCODE_HEIGHT;
    /* frames per second */
    AVRational timeBase = {1, atoi(StreamConfig::video_framerate_val().c_str())};
    mCodecCtx->time_base = timeBase;
    AVRational framerate = {atoi(StreamConfig::video_framerate_val().c_str()), 1};
    mCodecCtx->framerate = framerate;

    /* emit one intra frame every ten frames
    * check frame pict_type before passing frame
    * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
    * then gop_size is ignored and the output of encoder
    * will always be I frame irrespective to gop_size
    */
    mCodecCtx->gop_size = 30;
    mCodecCtx->max_b_frames = 1;
    mCodecCtx->keyint_min = 15;
    mCodecCtx->pix_fmt = DEFAULT_ENCODE_FORMAT;
    //mCodecCtx->lumi_masking = 0;
    //mCodecCtx->dark_masking = 0;
    //mCodecCtx->dct_algo = 1;
    mCodecCtx->me_range = 0;
    mCodecCtx->qcompress = 0;
    //mCodecCtx->thread_count = 5;
    mCodecCtx->refs = 5;

    ret = avcodec_open2(mCodecCtx, mCodec, NULL);
    if (ret < 0) {
        printf("avcodec_open2 error, ret: %d\n", ret);
        avcodec_free_context(&mCodecCtx);
        av_packet_free(&mPkt);

        //char *err = av_err2str(ret);
        //fprintf(stderr, "Could not open codec: %s\n", err);
        return false;
    }

    mFrame = av_frame_alloc();
    if (!mFrame) {
        fprintf(stderr, "Could not allocate video frame\n");
        avcodec_free_context(&mCodecCtx);
        av_packet_free(&mPkt);

        return false;
    }
    mFrame->format = mCodecCtx->pix_fmt;
    mFrame->width  = mCodecCtx->width;
    mFrame->height = mCodecCtx->height;

    ret = av_frame_get_buffer(mFrame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        avcodec_free_context(&mCodecCtx);
        av_packet_free(&mPkt);
        av_frame_free(&mFrame);

        return false;
    }

    return true;
}

bool Mpeg1VideoEncoder::closeEncoder(){
    if(mCodecCtx){
        avcodec_close(mCodecCtx);
        avcodec_free_context(&mCodecCtx);
    }
    av_packet_free(&mPkt);
    av_frame_free(&mFrame);

    return true;
}

AVCodecContext *Mpeg1VideoEncoder::getCodecContex(){
    return mCodecCtx;
}

