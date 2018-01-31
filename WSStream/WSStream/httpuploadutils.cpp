#include "httpuploadutils.h"
#include <QFile>
#include <QDataStream>
#include <QVariant>
#include <QUuid>
#include <QHttpPart>

HttpUploadUtils::HttpUploadUtils(QObject *parent) : QObject(parent)
{
    mJpegStream = NULL;
    uploadManager = NULL;
    multiPart = NULL;
    _reply = NULL;
}

void HttpUploadUtils::msgToList(AVFrame *frame){
    mListLock.lock();
    list.push_back(frame);
    mListLock.unlock();
}

void HttpUploadUtils::msgFromList(AVFrame **frame){
    mListLock.lock();
    *frame = list.front();
    list.pop_front();
    mListLock.unlock();
}

bool HttpUploadUtils::makeJpeg(){
    AVFrame *frame = NULL;
    int msgSize = 0, errCode = 0;
    bool ret = true;

    msgSize = list.size();
#if 1
    qDebug() << "before size: " << msgSize;
#endif
    if(msgSize == 0){
        return false;
    }
    msgFromList(&frame);
    if(!frame){
        qDebug() << "msgFromList error";
        return false;
    }

    msgSize = list.size();
#if 1
    qDebug() << "after size: " << msgSize;
#endif

    if(!mJpegStream){
        mJpegStream = new JpegUtils();
        if(!mJpegStream){
            qDebug() << "memory low error";
            av_frame_free(&frame);
            frame = NULL;
            return false;
        }else{
            ret = mJpegStream->init(true);
            if(!ret){
               qDebug() << "jpeg stream init fail";
               delete mJpegStream;
               mJpegStream = NULL;
               av_frame_free(&frame);
               frame = NULL;
               return false;
            }
        }

        //获取文件名
        uploadFilename = QString(StreamConfig::jpeg_file_name_val().c_str());
        qDebug() << "uploadFilename: " << uploadFilename;
        mJpegStream->writeOneFrameToStream(frame, AVMEDIA_TYPE_VIDEO, &errCode);
        //qDebug() << "writeOneFrameToStream";
        //delete mJpegStream;
        mJpegStream->deleteLater();
        mJpegStream = NULL;
    }
    //qDebug() << "av_frame_free before";
    //释放内存
    av_frame_free(&frame);
    frame = NULL;
   // qDebug() << "av_frame_free after";
    return true;
}

void HttpUploadUtils::upLoadFileByPost(){
    bool ret = true;
    //生成jpg
    ret = makeJpeg();
    if(!ret){
        qDebug() << "makeJpeg fail";
        return;
    }

    if(!uploadManager) {
        uploadManager = new QNetworkAccessManager();
        if(uploadManager)
            connect(uploadManager,SIGNAL(finished(QNetworkReply*)),SLOT(replyFinished(QNetworkReply*)));
    }
    //qDebug() <<  "111111111111111111111111111111";
    if(!uploadManager){
        qDebug() << "QNetworkAccessManager is NULL error.";
        return;
    }
#if 1
    //如果post请求失败 释放内存
    if(_reply){
        _reply->deleteLater();
        _reply = Q_NULLPTR;
    }
   // qDebug() << "222222222222222222222222222";
    if(multiPart) {
        multiPart->deleteLater();
        multiPart = Q_NULLPTR;
    }
    multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    if(!multiPart)
        return;
   // qDebug() << "33333333333333333333";
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
    QFile *file = new QFile(uploadFilename);
    if(file){
        if(!file->open(QIODevice::ReadOnly)){
            delete multiPart;
            return;
        }
        QByteArray sb = QByteArray();
        sb.append(file->readAll());
        file->close();
        imagePart.setBody(sb);
    }
    else{
        qDebug() << "open file fail.";
        if(multiPart) {
             multiPart->deleteLater();
             multiPart = Q_NULLPTR;
        }
        return;
    }
    //qDebug() << "4444444444444444444444444";
    QString qurl = QString(HTTP_SVR_ULR).arg(StreamConfig::http_svr_addr_val().c_str()).arg(StreamConfig::http_svr_port_val()).arg(StreamConfig::jpeg_file_name_val().c_str()).arg(StreamConfig::machine_code_val().c_str());
    string url = qurl.toStdString();
    qDebug() << "url: " << url.c_str();
    QNetworkRequest request(QUrl(url.c_str()));
    //QUrl url = QUrl("http://www.91sweep.com/files/api/upload/update_image.action?img_name=123.jpg&machine_code=machine_code_2000_1");
    //QNetworkRequest request(url);
    multiPart->append(imagePart);
    _reply = uploadManager->post(request,multiPart);
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(upLoadError(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(uploadProgress ( qint64 ,qint64 )), this, SLOT( OnUploadProgress(qint64 ,qint64 )));
#endif
}

void HttpUploadUtils::replyFinished(QNetworkReply* reply){
   qDebug() << "HttpUploadThread::replyFinished: ";
   if(reply->error() == QNetworkReply::NoError)
   {
       qDebug() << "no error.....";
       QByteArray bytes = reply->readAll();  //获取字节
       QString result(bytes);  //转化为字符串
       qDebug() << result;
   }
   else{
       qDebug() << "replyFinished:  " << reply->error() << " " <<reply->errorString();
   }

   QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
   qDebug() << status_code;

   if(multiPart) {
        multiPart->deleteLater();
        multiPart = Q_NULLPTR;
   }

   if(_reply){
       _reply->deleteLater();
       _reply = Q_NULLPTR;
   }
}

void HttpUploadUtils::upLoadError(QNetworkReply::NetworkError errorCode)
{
   qDebug() << "upLoadError  errorCode: " << (int)errorCode;
}

void HttpUploadUtils::OnUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
   qDebug() << "bytesSent: " << bytesSent << "  bytesTotal: "<< bytesTotal;
}
