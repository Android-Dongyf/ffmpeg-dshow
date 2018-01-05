#ifndef MP2AUDIOENCODER_H
#define MP2AUDIOENCODER_H
#include "encoder.h"

#define DEFAULT_AUDIO_ENCODE_NAME "mp2"
#define DEFAULT_AUDIO_ENCODE_BITRATE 128000
//#define DEFAULT_AUDIO_SAMPLE_RATE 32000
//#define DEFAULT_AUDIO_CHANNELS 2

class Mp2AudioEncoder : public encoder
{
public:
    Mp2AudioEncoder();
    ~Mp2AudioEncoder();

    virtual bool init();
    virtual bool encode(AVFrame *frame, AVPacket *pkt);
    virtual AVCodecContext *getCodecContex();

private:
    virtual bool openEncoder();
    virtual bool closeEncoder();

private:
    AVCodec *mCodec;
    AVCodecContext *mCodecCtx;

    struct SwrContext *mSwrAudioCtx;
    int samples_count;
};

#endif // MP2AUDIOENCODER_H
