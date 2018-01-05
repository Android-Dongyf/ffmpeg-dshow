#ifndef HTTPUPLOADUTILS_H
#define HTTPUPLOADUTILS_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QMutex>
#include <QList>

#include "ffmpegall.h"
#include "jpegutils.h"

#define HTTP_SVR_ULR "http://%1:%2/files/api/upload/update_image.action?img_name=%3&machine_code=%4"

class HttpUploadUtils : public QObject
{
    Q_OBJECT
public:
    explicit HttpUploadUtils(QObject *parent = 0);
    void msgToList(AVFrame *frame);
    void msgFromList(AVFrame **frame);
private:
    bool makeJpeg();
signals:

public slots:
    void replyFinished(QNetworkReply*);
    void upLoadError(QNetworkReply::NetworkError errorCode);
    void OnUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void upLoadFileByPost();
private:
    QMutex mListLock;
    QList<AVFrame *> list;

    QString uploadFilename;
    JpegUtils *mJpegStream;

    QNetworkAccessManager *uploadManager;
    QNetworkReply *_reply;
    QHttpMultiPart *multiPart;
    //char* m_buf = NULL;
};

#endif // HTTPUPLOADUTILS_H
