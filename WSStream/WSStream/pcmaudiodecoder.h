#ifndef PCMAUDIODECODER_H
#define PCMAUDIODECODER_H
#include "decoder.h"

class PcmAudioDecoder : public decoder
{
public:
    PcmAudioDecoder();
    ~PcmAudioDecoder();

    virtual bool init(AVFormatContext *fmtCtx);
    virtual bool decode(AVPacket *pkt, AVFrame *frame);
private:
    virtual bool openDecoder(AVFormatContext *fmtCtx);
    virtual bool closeDecoder();
private:
    AVCodecContext *mCodeCtx;
    AVCodec *mCodec;
};

#endif // PCMAUDIODECODER_H
