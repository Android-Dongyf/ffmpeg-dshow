#ifndef MULTIAVSTREAM_H
#define MULTIAVSTREAM_H

#include <QMutex>
#include "outputstream.h"
#include "encoder.h"

//#define DEFAULT_OUTPUT_STREAM "http://192.168.1.3:8081/root"
//#define DEFAULT_OUTPUT_STREAM "http://47.96.153.251:8081/root/stream-1"
//#define DEFAULT_OUTPUT_STREAM "http://192.168.1.2/ffmpeg/upload_video.ffmpeg?stream_name=zhang"
//#define DEFAULT_OUTPUT_STREAM "http://192.168.1.23/stream/1"
//#define DEFAULT_OUTPUT_STREAM "http://192.168.1.6:9000/"
//#define DEFAULT_OUTPUT_FMT "mpegts"
//#define DEFAULT_STREAM_FRAMERATE 30

class MultiAVStream : public OutputStream
{
public:
    MultiAVStream();
    ~MultiAVStream();

    virtual bool init(bool isMaster);
    virtual bool openStream();
    virtual bool closeStream();
    virtual bool writeOneFrameToStream(AVFrame *frame, enum AVMediaType codec_type, int *err_code);
    virtual bool writeHeaderToStream();
    virtual bool getOutStreamStatus();
    virtual encoder *getAudioCodec();
private:
    void setSlaveOrMaster(bool isMaster);

    bool writeFrameToStream(AVPacket *pkt, enum AVMediaType codec_type);
private:
    AVOutputFormat *mOutFmt;
    AVFormatContext *mOutFmtCtx;
    AVStream *mOutStream;
    AVStream *mOutAudioStream;

    encoder *mVideoEncoder;
    encoder *mAudioEncoder;
    int64_t audioPts;

    QMutex mWriteLock;

    QMutex mStatusLock;
    bool mStatusOk;

    bool master;
};

#endif // MULTIAVSTREAM_H
