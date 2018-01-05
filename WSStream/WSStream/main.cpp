#include <QCoreApplication>
#include "streamingthread.h"
#include "httpuploadthread.h"
#include "streamconfig.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qDebug() << "main::mian : " << QThread::currentThreadId();

    StreamConfig streamConf;

    StreamingThread *thread = new StreamingThread();
    thread->start();

    return a.exec();
}
