#include "videocapturethread.h"
#include <QDebug>
#include <QDateTime>


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

void VideoCaptureThread::setInputStream(InputStream *in){
    mIn = in;
}

void VideoCaptureThread::setOutputStream(OutputStream *out){
    mOut = out;
}

void VideoCaptureThread::run(){
    qDebug() << "videoCaptureThread::run : " << QThread::currentThreadId();
    bool ret;
    int rc = 0;
    AVFrame *frame = NULL;

    //QThread::sleep(2);

    while(true) {
        frame = av_frame_alloc();
        if(frame == NULL){
            qDebug() << "VideoCaptureThread frame == NULL";
            continue;
        }
        //unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val()));
        //avpicture_fill((AVPicture *)frame, out_buffer, AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val());
        frame->format = AV_PIX_FMT_YUV420P;
        frame->width  = StreamConfig::video_width_val();
        frame->height = StreamConfig::video_height_val();
        rc = av_frame_get_buffer(frame, 32);
        if(rc < 0){
            qDebug() << "VideoCaptureThread error.";
            QThread::msleep(30);
            continue;
        }

        rc = av_frame_make_writable(frame);
        if(rc < 0){
            qDebug() << "VideoCaptureThread  frame is not writable.";
            av_frame_free(&frame);
            frame = NULL;
            QThread::msleep(30);
            continue;
        }

        ret = mIn->getOneFrameFromStream(frame);
        if(!ret){
            //qDebug() << "VideoCaptureThread getOneFrameFromStream fail.";
            QThread::msleep(30);
            av_frame_free(&frame);
            frame = NULL;
            continue;
        }
       // qDebug() << "w: " << frame->width << " h: " << frame->height;

        upLoadIfNeed(frame);

        if(mOut->getOutStreamStatus()) {
            int errCode = 0;
            ret = mOut->writeOneFrameToStream(frame, mIn->getCodecMediaType(), &errCode);
            if(!ret)
                QThread::msleep(30);
        }

        av_frame_free(&frame);
        frame = NULL;
    }
}

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
    AVFrame *bakFrame = frame;

    jpgframe = av_frame_alloc();
    //unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val()));
    //avpicture_fill((AVPicture *)jpgframe, out_buffer, AV_PIX_FMT_YUV420P, StreamConfig::video_width_val(), StreamConfig::video_height_val());

    frame->buf[0] = NULL;
    av_frame_ref(jpgframe, frame);
    frame = bakFrame;

    //传数据 发信号
    mHttpUplaodUtils.msgToList(jpgframe);
    emit sigUploadFile();
}
