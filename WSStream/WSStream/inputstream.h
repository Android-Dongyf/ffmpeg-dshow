#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <QObject>
#include "ffmpegall.h"
#include "common.h"
#include <QMutex>
class InputStream : public QObject
{
    Q_OBJECT
public:
    explicit InputStream(QObject *parent = 0);

    virtual bool init() = 0;
    virtual void setDeviceLock(QMutex *lock) = 0;
    virtual bool getOneFrameFromStream(AVFrame *frame) = 0;

    virtual bool initFifo(AVCodecContext *output_codec_context) = 0;
    virtual int getFifoSize() = 0;
    virtual bool addSampleToFifo(AVFrame *frame) = 0;
    virtual bool readSampleFromFifo(AVFrame *frame, AVCodecContext *output_codec_context) = 0;
    virtual bool readSampleFromFifo1(AVFrame **frame, AVCodecContext *output_codec_context) = 0;
    virtual enum AVMediaType getCodecMediaType() = 0;
private:
    virtual bool openStream() = 0;
    virtual bool closeStream() = 0;

signals:

public slots:
};

#endif // INPUTSTREAM_H
