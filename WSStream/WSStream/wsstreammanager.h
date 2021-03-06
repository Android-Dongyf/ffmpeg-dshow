#ifndef WSSTREAMMANAGER_H
#define WSSTREAMMANAGER_H

#include <QObject>
#include <common.h>
#include <ffmpegmanager.h>
#include <QMutex>
#include <QList>

#include "videoinputstream.h"
#include "audioinputstream.h"
#include "multiavstream.h"
#include "videocapturethread.h"


class WSStreamManager : public QObject
{
    Q_OBJECT
public:
    explicit WSStreamManager(QObject *parent = 0);
    ~WSStreamManager();

    void start();
private:
    void init();
    bool frameToList(AVFrame *frame);
signals:

public slots:

private:
    FfmpegManager mFfmpegManager;

    InputStream *mInputStream;
    InputStream *mInputStream1;
    OutputStream *mOutputStream;
    OutputStream *mOutputStream1;
    VideoCropFilter *videoCrop;
    VideoCaptureThread *videoCapThread;
    QList<AVFrame *> frameList;
    QMutex listLock;
};

#endif // WSSTREAMMANAGER_H
