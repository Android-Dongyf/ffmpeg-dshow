#ifndef DECODER_H
#define DECODER_H
#if _MSC_VER
#define snprintf _snprintf
#endif
#include <QObject>
#include "ffmpegall.h"

class decoder : public QObject
{
    Q_OBJECT
public:
    explicit decoder(QObject *parent = 0);
    virtual bool init(AVFormatContext *fmtCtx) = 0;
    virtual bool decode(AVPacket *pkt, AVFrame *frame) = 0;
private:
    virtual bool openDecoder(AVFormatContext *fmtCtx) = 0;
    virtual bool closeDecoder() = 0;
signals:

public slots:
};

#endif // DECODER_H
