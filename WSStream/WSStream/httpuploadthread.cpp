#include "httpuploadthread.h"
#include <QFile>
#include <QDataStream>
#include <QVariant>

HttpUploadThread::HttpUploadThread(QObject *parent)
:QThread(parent)
{
}

void HttpUploadThread::run(){
    QThread::exec();
}



