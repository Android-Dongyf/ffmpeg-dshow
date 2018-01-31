#ifndef RAWVIDEODECODER_H
#define RAWVIDEODECODER_H

#include "decoder.h"
#include "videocropfilter.h"

class RawVideoDecoder : public decoder
{
public:
    RawVideoDecoder();
    ~RawVideoDecoder();

    virtual bool init(AVFormatContext *fmtCtx);
    virtual bool decode(AVPacket *pkt, AVFrame *frame);
private:
    virtual bool openDecoder(AVFormatContext *fmtCtx);
    virtual bool closeDecoder();

private:
    AVCodecContext *mCodeCtx;
    AVCodec *mCodec;
    struct SwsContext *mCvtCtx;
    AVFrame *mFrame420;
    AVFrame *mFrame422;
    VideoCropFilter *videoCrop;
};

#endif // RAWVIDEODECODER_H
