#ifndef AUDIOINPUTSTREAM_H
#define AUDIOINPUTSTREAM_H

#include "common.h"
#include "inputstream.h"
#include "pcmaudiodecoder.h"
#include <QMutex>

//#define AUDIO_INPUT_FORMAT_DESC "dshow"
//#define AUDIO_INPUT_DEVICE_DESC "audio=麦克风 (HD Pro Webcam C920)"
//#define AUDIO_INPUT_DEVICE_DESC "audio=麦克风阵列 (Full HD 1080P PC Camera"
//#define AUDIO_INPUT_DEVICE_DESC "audio=麦克风 (USB2.0 MIC)"
//#define AUDIO_INPUT_CHANNEL 2
//#define AUDIO_INPUT_SAMPLE_RATE 32000


class AudioInputStream : public InputStream
{
public:
    AudioInputStream();
    ~AudioInputStream();

    virtual bool init();
    virtual void setDeviceLock(QMutex *lock);
    virtual bool getOneFrameFromStream(AVFrame *frame);
    virtual bool initFifo(AVCodecContext *output_codec_context);
    virtual int getFifoSize();
    virtual bool addSampleToFifo(AVFrame *frame);
    virtual bool readSampleFromFifo(AVFrame *frame, AVCodecContext *output_codec_context);
    virtual bool readSampleFromFifo1(AVFrame **frame, AVCodecContext *output_codec_context);
    virtual bool destroyFifo();
    virtual enum AVMediaType getCodecMediaType();
    virtual AVFormatContext *getInputFormatCtx();
private:
    virtual bool openStream();
    virtual bool closeStream();
    bool initAudioFifo(AVCodecContext *output_codec_context);
private:
    AVInputFormat *mIfmt;
    AVFormatContext *mIfmtCtx;

    decoder *mCodecDecoder;
    AVAudioFifo *mAudioFifo;

    enum AVMediaType mCodecMediaType;

    QMutex *deviceLock;
};

#endif // AUDIOINPUTSTREAM_H
