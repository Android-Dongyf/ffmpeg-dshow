#include "wsstreammanager.h"
#include <QThread>
#include <QDateTime>
#include <QMutexLocker>


WSStreamManager::WSStreamManager(QObject *parent) : QObject(parent)
,mFfmpegManager(this)
{
     mInputStream = NULL;
     mOutputStream = NULL;
     mOutputStream1 = NULL;
     videoCrop = NULL;
     videoCapThread = NULL;
     init();
}

WSStreamManager::~WSStreamManager() {
    qDebug() << "WSStreamManager::~WSStreamManager";
    if(mInputStream)
        mInputStream->deleteLater();

     qDebug() << "WSStreamManager::~WSStreamManager1";
    if(mOutputStream)
        mOutputStream->deleteLater();
     qDebug() << "WSStreamManager::~WSStreamManager2";
    if(mOutputStream1)
        mOutputStream1->deleteLater();
     qDebug() << "WSStreamManager::~WSStreamManager3";
    if(videoCapThread) {
        //videoCapThread->exit();
        videoCapThread->deleteLater();
    }
     qDebug() << "WSStreamManager::~WSStreamManager4";
}

void WSStreamManager::init() {
    qDebug() << "WSStreamManager::init";

    mInputStream = new VideoInputStream();
    if(!mInputStream)
        return;

    mOutputStream = new MultiAVStream();
    if(!mOutputStream)
        return;

    mOutputStream1 = new MultiAVStream();
    if(!mOutputStream1)
        return;

    videoCapThread = new VideoCaptureThread();
    if(!videoCapThread)
        return;

}

void WSStreamManager::start() {
     qDebug() << "WSStreamManager::start";
     bool ret = true;
     int rc = 0;
     QMutex dshowLock;
     AVFrame *frame = NULL;

     unsigned char *out_buffer = NULL;

     if(!mInputStream || !mOutputStream || !mOutputStream1)
         return;

     //初始化视频输入
     mInputStream->setDeviceLock(&dshowLock);
     ret = mInputStream->init();
     qDebug() << "mInputStream init " << ret;
     if(!ret)
         return;

     while(true) {
         //初始化输出
         ret = mOutputStream->init(false);
         if(ret)
             break;
         qDebug() << "need exit";
         mOutputStream->closeStream();
     }

     videoCapThread->setOutputStream(mOutputStream1);
     videoCapThread->setFrameListAndLock(&frameList, &listLock);
     videoCapThread->start();

     frame = av_frame_alloc();
     if(frame == NULL){
         qDebug() << "VideoCaptureThread frame == NULL";
         return;
     }
     out_buffer = (unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, StreamConfig::video_crop_width_val(), StreamConfig::video_crop_height_val()));
     if(!out_buffer){
         qDebug() << "out_buffer == NULL";
     }
     rc = avpicture_fill((AVPicture *)frame, out_buffer, AV_PIX_FMT_YUV420P, StreamConfig::video_crop_width_val(), StreamConfig::video_crop_height_val());
     if(rc < 0){
          qDebug() << "avpicture_fill fail";
     }

     while(true){
        if(mOutputStream->getOutStreamStatus()) {
             ret = mInputStream->getOneFrameFromStream(frame);
             if(!ret){
                 //qDebug() << "VideoCaptureThread getOneFrameFromStream fail.";
                 //QThread::msleep(30);
                 continue;
             }

             AVFrame *tmpFrame = av_frame_clone(frame);
             if(tmpFrame != NULL){
                 frameToList(tmpFrame);
                 tmpFrame = NULL;
             }

             int errCode = 0;
             ret = mOutputStream->writeOneFrameToStream(frame, mInputStream->getCodecMediaType(), &errCode);
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

     if(out_buffer){
         av_free(out_buffer);
         out_buffer = NULL;
     }
     if(frame){
         av_frame_free(&frame);
         frame = NULL;
     }
}

bool WSStreamManager::frameToList(AVFrame *frame){
    QMutexLocker lock(&listLock);
    frameList.push_back(frame);

    return true;
}
