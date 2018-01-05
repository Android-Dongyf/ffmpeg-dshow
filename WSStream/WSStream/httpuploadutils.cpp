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
#if Debug
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
#if Debug
    qDebug() << "after size: " << msgSize;
#endif

    if(!mJpegStream){
        mJpegStream = new JpegUtils();
        if(!mJpegStream){
            qDebug() << "memory low error";
            return false;
        }else{
            ret = mJpegStream->init();
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
        //qDebug() << "uploadFilename: " << uploadFilename;
        mJpegStream->writeOneFrameToStream(frame, AVMEDIA_TYPE_VIDEO, &errCode);

        delete mJpegStream;
        mJpegStream = NULL;

        //释放内存
        av_frame_free(&frame);
        frame = NULL;
    }

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

    if(!uploadManager){
        qDebug() << "QNetworkAccessManager is NULL error.";
        return;
    }
#if 1

    multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    if(!multiPart)
        return;
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
    QFile *file = new QFile(uploadFilename);
    if(!file->open(QIODevice::ReadOnly)){
        delete multiPart;
        return;
    }
    QByteArray sb = QByteArray();
    sb.append(file->readAll());
    file->close();
    imagePart.setBody(sb);
    //imagePart.setBodyDevice(file);
    //file->setParent(multiPart); // 现在不能删除文件，所以用 multiPart 删除
    QUrl url = QUrl("http://www.91sweep.com/files/api/upload/update_image.action?img_name=123.jpg&machine_code=machine_code_2000_1");
    QNetworkRequest request(url);
    multiPart->append(imagePart);
    _reply = uploadManager->post(request,multiPart);
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(upLoadError(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(uploadProgress ( qint64 ,qint64 )), this, SLOT( OnUploadProgress(qint64 ,qint64 )));
#endif
#if 0
    //post request
    QString BOUNDARY = QUuid::createUuid().toString();
    QByteArray sb=QByteArray();

  /*  //先上传普通的表单数据
    QMap<QString,QString> params_send;
    params_send.insert("machine_code","machine_code_2000_1");
    for(QMap<QString,QString>::Iterator t=params_send.begin();t!=params_send.end();t++){

           sb.append("--"+BOUNDARY+"\r\n");

           sb.append(QString("Content-Disposition: form-data; name=\"")+t.key()+QString("\"")+QString("\r\n"));

           sb.append("\r\n");

           sb.append(t.value()+"\r\n");

    }*/

    //上传文件的头部
    sb.append("--"+BOUNDARY+"\r\n");
    sb.append(QString("Content-Disposition: form-data; name=\"image")+QString("\"; filename=\"")+uploadFilename+QString("\"")+QString("\r\n"));
    sb.append("\r\n");

    //上传文件内容
    QFile uploadFile(uploadFilename);
    if(!uploadFile.open(QIODevice::ReadOnly)){
        return;
    }
    sb.append(uploadFile.readAll());
    sb.append("\r\n");
    sb.append("--"+BOUNDARY+"\r\n");
    uploadFile.close();

    //编辑HTTP头部
    QString qurl = QString(HTTP_SVR_ULR).arg(StreamConfig::stream_output_addr_val().c_str()).arg(StreamConfig::http_svr_port_val()).arg(StreamConfig::jpeg_file_name_val().c_str()).arg(StreamConfig::machine_code_val().c_str());
    QNetworkRequest request(QUrl("http://www.91sweep.com/files/api/upload/update_image.action?img_name=123.jpg&machine_code=machine_code_2000_1"));
    string url = qurl.toStdString();
    qDebug() << "url: " << url.c_str();
    //QNetworkRequest request(QUrl(url.c_str()));
    request.setRawHeader(QString("Content-Type").toUtf8(),QString("multipart/form-data;boundary="+BOUNDARY).toUtf8());
    request.setRawHeader(QString("Content-Length").toUtf8(),QString::number(sb.length()).toUtf8());
    //qDebug() << "len: " << sb.length();
    _reply = uploadManager->post(request, sb);
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(upLoadError(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(uploadProgress ( qint64 ,qint64 )), this, SLOT( OnUploadProgress(qint64 ,qint64 )));
#endif
#if 0
    QString BOUNDARY = QUuid::createUuid().toString();
    QFile file(uploadFilename);
    file.open(QIODevice::ReadOnly);
    int file_len = file.size();
    QDataStream in(&file);
    m_buf = new char[file_len];
    in.readRawData( m_buf, file_len);
    file.close();
    qDebug() << "file_len: " << file_len;
    QNetworkRequest request(QUrl("http://47.96.153.251:80/files/api/upload/update_image.action?img_name=stream-1.jpg&machine_code=machine_code_2000_1"));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
    request.setRawHeader(QString("Content-Type").toUtf8(),QString("multipart/form-data;boundary="+BOUNDARY).toUtf8());
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");//这段没有manager返回错误203
    request.setRawHeader("filename", uploadFilename.section('/', -1, -1).toUtf8() );

    QByteArray arr = QByteArray(m_buf, file_len);
    _reply = uploadManager->post(request, arr);
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(upLoadError(QNetworkReply::NetworkError)));
    connect(_reply, SIGNAL(uploadProgress ( qint64 ,qint64 )), this, SLOT( OnUploadProgress(qint64 ,qint64 )));
   // if(uploadManager){
     //   delete uploadManager;
       // uploadManager = NULL;
    //}
#endif

}

void HttpUploadUtils::replyFinished(QNetworkReply* reply){
   //qDebug() << "HttpUploadThread::replyFinished: ";
   if(reply->error() == QNetworkReply::NoError)
   {
       //qDebug() << "no error.....";
       //QByteArray bytes = reply->readAll();  //获取字节
       //QString result(bytes);  //转化为字符串
       //qDebug() << result;
   }
   else{
       //qDebug() << "replyFinished:  " << reply->error() << " " <<reply->errorString();
   }

   //QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
   //qDebug() << status_code;

   if(multiPart) {
        delete multiPart;
        multiPart = NULL;
   }

   if(_reply){
       _reply->deleteLater();
       _reply = Q_NULLPTR;
   }
}

void HttpUploadUtils::upLoadError(QNetworkReply::NetworkError errorCode)
{
   //qDebug() << "upLoadError  errorCode: " << (int)errorCode;
}

void HttpUploadUtils::OnUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
   //qDebug() << "bytesSent: " << bytesSent << "  bytesTotal: "<< bytesTotal;
}
