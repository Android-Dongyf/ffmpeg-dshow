#ifndef MPEG1VIDEOENCODER_H
#define MPEG1VIDEOENCODER_H

#include "encoder.h"

#define DEFAULT_ENCODE_NAME "mpeg1video"
//#define DEFAULT_ENCODE_FRAMERATE 30
#define DEFAULT_ENCODE_BITRATE 1500000
//#define DEFAULT_ENCODE_WIDTH 352
//#define DEFAULT_ENCODE_HEIGHT 288
#define DEFAULT_ENCODE_FORMAT AV_PIX_FMT_YUV420P




class Mpeg1VideoEncoder : public encoder
{
public:
    Mpeg1VideoEncoder();
    ~Mpeg1VideoEncoder();

    virtual bool init();
    virtual bool encode(AVFrame *frame, AVPacket *pkt);

    virtual AVCodecContext *getCodecContex();
private:
    virtual bool openEncoder();
    virtual bool closeEncoder();

private:
    AVCodec *mCodec;
    AVCodecContext *mCodecCtx;
    AVFrame *mFrame;
    AVFrame *mTmpFrame;
    AVPacket *mPkt;

};

#endif // MPEG1VIDEOENCODER_H
