#include "streamingthread.h"
#include <QDebug>

StreamingThread::StreamingThread(QObject *parent)
:QThread(parent)
{
    //pWs = NULL;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    qDebug() << "StreamingThread::StreamingThread : " << QThread::currentThreadId();
}

void StreamingThread::run(){
    qDebug() << "StreamingThread::run : " << QThread::currentThreadId();

    //while(true){
        //pWs = new WSStreamManager();
        //pWs->start();

        //delete pWs;
        //pWs = NULL;
   // }
    ws.start();
}

