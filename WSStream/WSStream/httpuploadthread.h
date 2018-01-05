#ifndef HTTPUPLOADTHREAD_H
#define HTTPUPLOADTHREAD_H

#include <QThread>
#include <QMutex>
#include <QList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "ffmpegall.h"
#include "jpegutils.h"

class HttpUploadThread : public QThread
{
    Q_OBJECT
public:
    explicit HttpUploadThread(QObject *parent = 0);

protected:
    virtual void run() Q_DECL_OVERRIDE;

};

#endif // HTTPUPLOADTHREAD_H
