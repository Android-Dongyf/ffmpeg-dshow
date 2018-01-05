#include "streamingthread.h"
#include <QDebug>

StreamingThread::StreamingThread(QObject *parent)
:QThread(parent)
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    qDebug() << "StreamingThread::StreamingThread : " << QThread::currentThreadId();
}

void StreamingThread::run(){
    qDebug() << "StreamingThread::run : " << QThread::currentThreadId();

    ws.start();
}

