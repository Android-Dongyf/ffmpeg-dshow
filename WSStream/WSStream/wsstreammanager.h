#ifndef WSSTREAMMANAGER_H
#define WSSTREAMMANAGER_H

#include <QObject>
#include <common.h>
#include <ffmpegmanager.h>

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
signals:

public slots:

private:
    FfmpegManager mFfmpegManager;

    InputStream *mInputStream;
    InputStream *mInputStream1;
    OutputStream *mOutputStream;

    VideoCaptureThread *videoCapThread;
};

#endif // WSSTREAMMANAGER_H
