#ifndef ENCODER_H
#define ENCODER_H

#include <QObject>
#include "ffmpegall.h"
#include "common.h"
class encoder : public QObject
{
    Q_OBJECT
public:
    explicit encoder(QObject *parent = 0);
    virtual bool init() = 0;
    virtual bool encode(AVFrame *frame, AVPacket *pkt) = 0;
    virtual AVCodecContext *getCodecContex() = 0;
private:
    virtual bool openEncoder() = 0;
    virtual bool closeEncoder() = 0;
signals:

public slots:
};

#endif // ENCODER_H
