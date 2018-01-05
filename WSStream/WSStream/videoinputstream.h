#ifndef VIDEOINPUTSTREAM_H
#define VIDEOINPUTSTREAM_H

#include "inputstream.h"
#include <common.h>
#include "rawvideodecoder.h"
#include <QMutex>

//麦克风 (HD Pro Webcam C920)   audio

//#define INPUT_FORMAT_DESC "dshow"
//#define INPUT_DEVICE_DESC "video=Full HD 1080P PC Camera:audio=FrontMic (Realtek High Definition Audio)"
//#define INPUT_DEVICE_DESC "video=Full HD 1080P PC Camera"
//#define INPUT_DEVICE_DESC "video=HD Pro Webcam C920"
//#define INPUT_DEVICE_DESC "video=dummy"
//#define INPUT_DEVICE_DESC "video=USB2.0 PC CAMERA"
//#define DEFAULT_FRAMETATE "30"
//#define DEFAULT_CAPTURE_SIZE "352x288"
//#define DEFAULT_CAPTURE_FMT "yuyv422"

//#define DEFAULT_SAMPLE_RATE 44100
//#define DEFAULT_SAMPLE_CHANNELS 2


class VideoInputStream : public InputStream
{
public:
    VideoInputStream();
    ~VideoInputStream();

    virtual bool init();
    virtual void setDeviceLock(QMutex *lock);
    virtual bool getOneFrameFromStream(AVFrame *frame);
    virtual bool initFifo(AVCodecContext *output_codec_context);
    virtual int getFifoSize();
    virtual bool addSampleToFifo(AVFrame *frame);
    virtual bool readSampleFromFifo(AVFrame *frame, AVCodecContext *output_codec_context);
    virtual bool readSampleFromFifo1(AVFrame **frame, AVCodecContext *output_codec_context);
    virtual enum AVMediaType getCodecMediaType();
private:
    virtual bool openStream();
    virtual bool closeStream();


    int init_input_threads(void);
    //void *input_thread(void *arg);
    int get_input_packet_mt(AVPacket *pkt);
    void free_input_threads(void);
private:
    AVInputFormat *mIfmt;


    decoder *mCodecDecoder;

    enum AVMediaType mCodecMediaType;
public:
    AVFormatContext *mIfmtCtx;
    AVThreadMessageQueue *in_thread_queue;
    pthread_t thread;
    int non_blocking;           /* reading packets from the thread should not block */
    int thread_queue_size;      /* maximum number of queued packets */

    QMutex *deviceLock;
};

#endif // VIDEOINPUTSTREAM_H
