#ifndef JPEGUTILS_H
#define JPEGUTILS_H

#include "ffmpegall.h"
#include "outputstream.h"
#include "jpegencoder.h"

//#define DEFAULT_OUTPUT_FMT "singlejpeg"
//#define DEFAULT_OUTPUT_STREAM "./stream-1.jpg"

class JpegUtils : public OutputStream
{
public:
    JpegUtils();
    ~JpegUtils();

    virtual bool init(bool isMaster);
    virtual bool openStream();
    virtual bool closeStream();
    virtual bool writeOneFrameToStream(AVFrame *frame, enum AVMediaType codec_type, int *err_code);
    virtual bool writeHeaderToStream();
    virtual bool getOutStreamStatus();
    virtual encoder *getAudioCodec();
private:


    bool writeFrameToStream(AVPacket *pkt, enum AVMediaType codec_type);
private:
    AVOutputFormat *mOutFmt;
    AVFormatContext *mOutFmtCtx;
    AVStream *mOutStream;

    encoder *mEncoder;
};

#endif // JPEGUTILS_H
