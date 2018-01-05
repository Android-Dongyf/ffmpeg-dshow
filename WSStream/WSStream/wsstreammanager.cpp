#include "wsstreammanager.h"
#include <QThread>
#include <QDateTime>



WSStreamManager::WSStreamManager(QObject *parent) : QObject(parent)
,mFfmpegManager(this)
{
    init();
}

WSStreamManager::~WSStreamManager() {
    qDebug() << "WSStreamManager::~WSStreamManager";
    if(mInputStream)
        delete mInputStream;
    if(mInputStream1)
        delete mInputStream1;

    if(mOutputStream)
        delete mOutputStream;
}

void WSStreamManager::init() {
    qDebug() << "WSStreamManager::init";

    mInputStream = new VideoInputStream();
    if(!mInputStream)
        return;



    mInputStream1 = new AudioInputStream();
    if(!mInputStream1)
        return;



    mOutputStream = new MultiAVStream();
    if(!mOutputStream)
        return;

    videoCapThread = new VideoCaptureThread();
    if(!videoCapThread)
        return;

    //qDebug() << "WSStreamManager::init : " << QThread::currentThreadId();
}

void WSStreamManager::start() {
     qDebug() << "WSStreamManager::start";
     bool ret = true;
     AVFrame *output_frame = NULL, *audioFrame = NULL;
     QMutex dshowLock;

     if(!mInputStream || !mInputStream1 || !mOutputStream)
         return;

     //初始化视频输入
     mInputStream->setDeviceLock(&dshowLock);
     ret = mInputStream->init();
     qDebug() << "mInputStream init " << ret;
     if(!ret)
         return;

     //初始化音频输入
     mInputStream1->setDeviceLock(&dshowLock);
     ret = mInputStream1->init();
     qDebug() << "mInputStream1 init " << ret;
     if(!ret)
         return;

     while(1) {
         //初始化输出
         ret = mOutputStream->init();
         if(ret)
             break;
    }

     //初始化音频Fifo
     ret = mInputStream1->initFifo(mOutputStream->getAudioCodec()->getCodecContex());
     if(!ret){
         qDebug() << "initFifo fail.";
         return;
     }

    //写输出格式头部信息
    /* ret = mOutputStream->writeHeaderToStream();
     if(!ret)
         return;*/

     const int output_frame_size = mOutputStream->getAudioCodec()->getCodecContex()->frame_size;

     videoCapThread->setInputStream(mInputStream);
     videoCapThread->setOutputStream(mOutputStream);
     videoCapThread->start();

     while(false){
         audioFrame = av_frame_alloc();
        // qDebug() << "test run";
         av_frame_free(&audioFrame);
         QThread::sleep(1);
     }
    // audioFrame = av_frame_alloc();
    // output_frame = av_frame_alloc();

     while(true){
         audioFrame = av_frame_alloc();
         if(!audioFrame){
             qDebug() << "memoer low audioFrame == NULL";
             continue;
         }
         qDebug() <<"mInputStream1 11";
#if 1
        while(mInputStream1->getFifoSize() < output_frame_size) {
            qDebug() <<"mInputStream1 22";
            ret = mInputStream1->getOneFrameFromStream(audioFrame);
            if(!ret){
                qDebug() << "mInputStream1 getOneFrameFromStream fail.";
                //goto end;
                break;
            }
            qDebug() <<"mInputStream1 33";
            ret = mInputStream1->addSampleToFifo(audioFrame);
            if(!ret){
                qDebug() << "mInputStream1 addSampleToFifo fail.";
                break;
            }
        }
        qDebug() <<"mInputStream1 44";
        if (mInputStream1->getFifoSize() >= output_frame_size){
             qDebug() <<"mInputStream1 55";
            ret = mInputStream1->readSampleFromFifo1(&output_frame, mOutputStream->getAudioCodec()->getCodecContex());
            if(!ret){
                qDebug() << "mInputStream1 readSampleFromFifo fail.";
                continue;
            }
             qDebug() <<"mInputStream1 66";
            if(mOutputStream->getOutStreamStatus()) {
                int errCode = 0;
                bool ret = mOutputStream->writeOneFrameToStream(output_frame, mInputStream1->getCodecMediaType(), &errCode);
                if(!ret) {
                    if(errCode == -2){
                        bool ret1 = true;
                        while(true) {
                            if(ret1)
                                mOutputStream->closeStream();
                            ret1 = mOutputStream->openStream();
                            if(ret1)
                                break;
                        }
                    }
                }
            }
        }
         qDebug() <<"mInputStream1 77";
#endif
        if(audioFrame){
             qDebug() <<"mInputStream1 7117";
            av_frame_free(&audioFrame);
        }
        else
            qDebug() <<"mInputStream1 99";
        if(output_frame){
            qDebug() <<"mInputStream1 8118";
            av_frame_free(&output_frame);
        }
        else
            qDebug() <<"mInputStream1 1010";
         qDebug() <<"mInputStream1 88";
     }

}
