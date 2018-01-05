#ifndef VIDEOCAPTURETHREAD_H
#define VIDEOCAPTURETHREAD_H

#include <QObject>
#include <QThread>
#include "inputstream.h"
#include "outputstream.h"
#include <QTimer>
#include "httpuploadutils.h"
#include "httpuploadthread.h"
class VideoCaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoCaptureThread(QObject *parent = 0);
    void setInputStream(InputStream *in);
    void setOutputStream(OutputStream *out);
    void setHttpThread(HttpUploadThread *thread);
private:
    void upLoadIfNeed(AVFrame *frame);
    void upLoadJpegWithFrame(AVFrame *frame);
protected:
    virtual void run() Q_DECL_OVERRIDE;

signals:
    void sigUploadFile();
private:
    InputStream *mIn;
    OutputStream *mOut;

    bool needUploadJpeg;

    HttpUploadThread mHttpThread;
    HttpUploadUtils mHttpUplaodUtils;

};

#endif // VIDEOCAPTURETHREAD_H
