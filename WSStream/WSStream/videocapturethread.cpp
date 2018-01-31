#include "videocapturethread.h"
#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>

VideoCaptureThread::VideoCaptureThread(QObject *parent)
:QThread(parent)
{
    needUploadJpeg = false;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    qDebug() << "videoCaptureThread::videoCaptureThread : " << QThread::currentThreadId();

    mHttpUplaodUtils.moveToThread(&mHttpThread);
    QObject::connect(this, SIGNAL(sigUploadFile()), &mHttpUplaodUtils, SLOT(upLoadFileByPost()));
    mHttpThread.start();
}

VideoCaptureThread::~VideoCaptureThread(){
    mHttpThread.exit();
}

void VideoCaptureThread::setOutputStream(OutputStream *out){
    mOut = out;
}

void VideoCaptureThread::setFrameListAndLock(QList<AVFrame *> *list, QMutex *lock){
    mList = list;
    mLock = lock;
}

void VideoCaptureThread::run(){
    qDebug() << "videoCaptureThread::run : " << QThread::currentThreadId();
    AVFrame *frame = NULL;
    bool ret = true;

    while(true) {
        //初始化输出
        ret = mOut->init(true);
        if(ret)
            break;
        qDebug() << "need exit";
        mOut->closeStream();
    }

    while(true) {
        if(mOut->getOutStreamStatus()) {
            ret = frameFromList(&frame);
            if(!ret){
                av_usleep(10000);
                continue;
            }
            if(frame){
                upLoadIfNeed(frame);

                int errCode = 0;
                ret = mOut->writeOneFrameToStream(frame, AVMEDIA_TYPE_VIDEO, &errCode);
                av_frame_free(&frame);
                frame = NULL;
                if(!ret) {
                   if(errCode == -2){
                       bool ret1 = true;
                       while(true) {
                           if(ret1)
                               mOut->closeStream();
                               ret1 = mOut->openStream();
                               if(ret1)
                                   break;
                       }
                    }
                }
            }
        }
    }
}

#if 0
void VideoCaptureThread::run(){
    qDebug() << "videoCaptureThread::run : " << QThread::currentThreadId();
    bool ret;
    int rc = 0;
    AVFrame *frame = NULL;
    unsigned char *out_buffer = NULL;
    frame = av_frame_alloc();
    if(frame == NULL){
        qDebug() << "VideoCaptureThread frame == NULL";
    }
    out_buffer = (unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val()));
    if(!out_buffer){
        qDebug() << "out_buffer == NULL";
    }
    rc = avpicture_fill((AVPicture *)frame, out_buffer, AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val());
    if(rc < 0){
         qDebug() << "avpicture_fill fail";
    }

    while(true) {
        if(mOut->getOutStreamStatus()) {
            ret = mIn->getOneFrameFromStream(frame);
            if(!ret){
                //qDebug() << "VideoCaptureThread getOneFrameFromStream fail.";
                QThread::msleep(30);
                continue;
            }

            upLoadIfNeed(frame);

            int errCode = 0;
            ret = mOut->writeOneFrameToStream(frame, mIn->getCodecMediaType(), &errCode);
            if(!ret)
                QThread::msleep(30);
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
#endif

void VideoCaptureThread::upLoadIfNeed(AVFrame *frame){
    static int cnt = 0, first = 1;

    cnt++;
    //qDebug() << "cnt: " << cnt;
    if((cnt > 10 && first) || (cnt > 1800)){
        cnt = first = 0;
        needUploadJpeg = true;
    }
    if(needUploadJpeg){
        needUploadJpeg = false;

        upLoadJpegWithFrame(frame);
    }
}

void VideoCaptureThread::upLoadJpegWithFrame(AVFrame *frame){
    AVFrame *jpgframe = NULL;

    jpgframe = av_frame_clone(frame);
    //qDebug() << "buf[0]: " << (frame->buf[0] ? "true" : "false");
    //av_frame_ref(jpgframe, frame);
    if(jpgframe) {
        //传数据 发信号
        mHttpUplaodUtils.msgToList(jpgframe);
        emit sigUploadFile();
    }
}

bool VideoCaptureThread::frameFromList(AVFrame **frame){
    if(!mLock || !mList || !frame)
        return false;

    *frame = NULL;
    QMutexLocker lock(mLock);
    //qDebug() << "VideoCaptureThread size: " << mList->size();
    if(mList->isEmpty())
        return false;

    *frame = mList->front();
    mList->pop_front();
    return true;
}
