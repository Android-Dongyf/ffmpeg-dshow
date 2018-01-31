#ifndef VIDEOCROPFILTER_H
#define VIDEOCROPFILTER_H

#include <QObject>
#include <ffmpegall.h>


#define FILTER_DESC "crop=640:1080:640:0,scale=352:288"

class VideoCropFilter : public QObject
{
    Q_OBJECT
public:
    explicit VideoCropFilter(QObject *parent = 0);
    ~VideoCropFilter();

    bool initWithInputFmtCtxAndDecCtx(AVFormatContext *ctx, AVCodecContext *dCtx);
    bool cropVideoWithFrame(AVFrame *srcFrame, AVFrame **dstFrame);
private:
    int initFilters(const char *filters_descr);
signals:

public slots:
private:
    AVFormatContext *iFmtCtx;
    AVCodecContext *decCtx;
    AVFilterContext *buffersinkCtx;
    AVFilterContext *buffersrcCtx;
    AVFilterGraph *filterGraph;
};

#endif // VIDEOCROPFILTER_H
