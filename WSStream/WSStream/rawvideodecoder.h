#ifndef RAWVIDEODECODER_H
#define RAWVIDEODECODER_H

#include "decoder.h"

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
    AVCodecParserContext *mParser;
    struct SwsContext *mCvtCtx;
    AVFrame *mFrame420;
    AVFrame *mFrame422;
};

#endif // RAWVIDEODECODER_H
