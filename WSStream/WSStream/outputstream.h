#ifndef OUTPUTSTREAM_H
#define OUTPUTSTREAM_H

#include <QObject>
#include "ffmpegall.h"
#include "encoder.h"

class OutputStream : public QObject
{
    Q_OBJECT
public:
    explicit OutputStream(QObject *parent = 0);

    virtual bool init(bool isMater) = 0;
    virtual bool openStream() = 0;
    virtual bool closeStream() = 0;
    virtual bool writeOneFrameToStream(AVFrame *frame, enum AVMediaType codec_type, int *err_code) = 0;
    virtual bool writeHeaderToStream() = 0;
    virtual bool getOutStreamStatus() = 0;
    virtual encoder *getAudioCodec() = 0;
private:


signals:

public slots:
};

#endif // OUTPUTSTREAM_H
