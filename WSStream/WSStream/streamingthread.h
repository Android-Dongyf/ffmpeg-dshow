#ifndef STREAMINGTHREAD_H
#define STREAMINGTHREAD_H

#include <QObject>
#include <QThread>
#include "wsstreammanager.h"


class StreamingThread : public QThread
{
    Q_OBJECT
public:
     explicit StreamingThread(QObject *parent = 0);
protected:
    virtual void run() Q_DECL_OVERRIDE;
private:
    WSStreamManager ws;
    //WSStreamManager *pWs;

};

#endif // STREAMINGTHREAD_H
