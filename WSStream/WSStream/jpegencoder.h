#ifndef JPEGENCODER_H
#define JPEGENCODER_H

#include "encoder.h"

#define DEFAULT_ENCODE_NAME "mjpeg"
//#define DEFAULT_ENCODE_WIDTH 352
//#define DEFAULT_ENCODE_HEIGHT 288
#define DEFAULT_ENCODE_FORMAT AV_PIX_FMT_YUVJ420P
class JpegEncoder : public encoder
{
public:
    JpegEncoder();
    ~JpegEncoder();

    virtual bool init();
    virtual bool encode(AVFrame *frame, AVPacket *pkt);

    virtual AVCodecContext *getCodecContex();
private:
    virtual bool openEncoder();
    virtual bool closeEncoder();
private:
    AVCodec *mCodec;
    AVCodecContext *mCodecCtx;
};

#endif // JPEGENCODER_H
