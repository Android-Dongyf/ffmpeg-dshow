#include "videoinputstream.h"
#include <QThread>

VideoInputStream::VideoInputStream(): InputStream(){

}

VideoInputStream::~VideoInputStream() {
    closeStream();
}

static void *input_thread(void *arg)
{
    VideoInputStream *f = (VideoInputStream *)arg;
    unsigned flags = f->non_blocking ? AV_THREAD_MESSAGE_NONBLOCK : 0;
    int ret = 0;
    AVPacket pkt;
    while (1) {
        ret = av_read_frame(f->mIfmtCtx, &pkt);
        if (ret == AVERROR(EAGAIN)) {
            qDebug() << "read again";
            av_usleep(10000);
            continue;
        }
        if (ret < 0) {
            av_thread_message_queue_set_err_recv(f->in_thread_queue, ret);
            break;
        }
        ret = av_thread_message_queue_send(f->in_thread_queue, &pkt, flags);
        if (flags && ret == AVERROR(EAGAIN)) {
            flags = 0;
            ret = av_thread_message_queue_send(f->in_thread_queue, &pkt, flags);
            av_log(f->mIfmtCtx, AV_LOG_WARNING,
                   "Thread message queue blocking; consider raising the "
                   "thread_queue_size option (current value: %d)\n",
                   f->thread_queue_size);
        }
        if (ret < 0) {
            if (ret != AVERROR_EOF) {
                av_log(f->mIfmtCtx, AV_LOG_ERROR,
                       "Unable to send packet to main thread. \n");
            }
            av_packet_unref(&pkt);
            av_thread_message_queue_set_err_recv(f->in_thread_queue, ret);
            break;
        }
    }

    return NULL;
}

int VideoInputStream::get_input_packet_mt(AVPacket *pkt)
{
    return av_thread_message_queue_recv(in_thread_queue, pkt,
                                        non_blocking ?
                                        AV_THREAD_MESSAGE_NONBLOCK : 0);
}

void VideoInputStream::free_input_threads(void)
{
    AVPacket pkt;

    if (!in_thread_queue)
        return;
    av_thread_message_queue_set_err_send(in_thread_queue, AVERROR_EOF);
    while (av_thread_message_queue_recv(in_thread_queue, &pkt, 0) >= 0)
        av_packet_unref(&pkt);

    pthread_join(thread, NULL);
    av_thread_message_queue_free(&in_thread_queue);
}

int VideoInputStream::init_input_threads(void)
{
    int ret;

    non_blocking = 1;
    thread_queue_size = 8;
    ret = av_thread_message_queue_alloc(&in_thread_queue, thread_queue_size, sizeof(AVPacket));
    if (ret < 0)
        return ret;

    if ((ret = pthread_create(&thread, NULL, input_thread, this))) {
        av_log(NULL, AV_LOG_ERROR, "pthread_create failed: %s. Try to increase `ulimit -v` or decrease `ulimit -s`.\n", strerror(ret));
        av_thread_message_queue_free(&in_thread_queue);
        return AVERROR(ret);
    }

    return 0;
}

bool VideoInputStream::init() {
    mCodecDecoder = new RawVideoDecoder();
    if(!mCodecDecoder)
        return false;

    bool ret = openStream();
    if(!ret)
        return false;

    /*if(init_input_threads() != 0) {
        printf("init_input_threads fail. \n");
        return false;
    }*/

    return true;
}

void VideoInputStream::setDeviceLock(QMutex *lock){
    deviceLock = lock;
}

bool VideoInputStream::openStream(){
    int ret = -1;
    AVDictionary *infmtOpt = NULL;
    //string frameRate(DEFAULT_FRAMETATE);
    //string videoSize(DEFAULT_CAPTURE_SIZE);

    string frameRate = StreamConfig::video_framerate_val();
    string videoSize = StreamConfig::video_size_str_val();

    int videoIndex = -1;
    //qDebug() << "fmt: " << StreamConfig::video_input_fmt_val().c_str();
    mIfmt = av_find_input_format(StreamConfig::video_input_fmt_val().c_str()/*INPUT_FORMAT_DESC*/);
    if(!mIfmt) {
        printf( "av_find_input_format fail.\n");
        return false;
    }

    if(mIfmt->priv_class){
        if(av_opt_find(&mIfmt->priv_class, "framerate", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)) {
            av_dict_set(&infmtOpt, "framerate", frameRate.c_str(), 0);
        }

        if(av_opt_find(&mIfmt->priv_class, "video_size", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set(&infmtOpt, "video_size", videoSize.c_str(), 0);
        }

        if(av_opt_find(&mIfmt->priv_class, "pixel_format", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set(&infmtOpt, "pixel_format", StreamConfig::video_fmt_val().c_str()/*DEFAULT_CAPTURE_FMT*/, 0);
        }
    }
#if Debug
    printf("video: framerate: %s video_size: %s\n",
           frameRate.c_str(), videoSize.c_str());
#endif

    //av_dict_set(&infmtOpt, "list_devices", "true", 0);
    //av_dict_set(&infmtOpt, "list_options ", "true", 0);

    mIfmtCtx = avformat_alloc_context();
    if(!mIfmtCtx){
        printf( "avformat_alloc_context fail.\n");
        return false;
    }

    if(av_opt_find(&mIfmtCtx->av_class, "rtbufsize", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
        av_dict_set_int(&infmtOpt, "rtbufsize", 184320000, 0);
    }

    if ((ret = avformat_open_input(&mIfmtCtx, StreamConfig::video_desc_val().c_str()/*INPUT_DEVICE_DESC*/, mIfmt, &infmtOpt)) < 0) {
        printf( "avformat_open_input fail.  ret: %d\n", ret);
        avformat_free_context(mIfmtCtx);
        return false;
    }

    if ((ret = avformat_find_stream_info(mIfmtCtx, NULL)) < 0) {
        printf( "Failed to retrieve input stream information");
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
        mIfmtCtx = NULL;
        return false;
    }

    if(mIfmtCtx){
#if Debug
        printf("nb_streams: %d\n",  mIfmtCtx->nb_streams);
#endif
    }
    else
        printf("mIfmtCtx == NULL\n");

    for(unsigned int i = 0; i < mIfmtCtx->nb_streams; i++)
        if(mIfmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            mCodecMediaType = AVMEDIA_TYPE_VIDEO;
            videoIndex = i;
            break;
        }
#if Debug
    printf("videoIndex: %d\n", videoIndex);
    printf("timebase  Numerator: %d Denominator: %d\n", mIfmtCtx->streams[videoIndex]->time_base.num, mIfmtCtx->streams[videoIndex]->time_base.den);
#endif

    av_dump_format(mIfmtCtx, 0, StreamConfig::video_desc_val().c_str()/*INPUT_DEVICE_DESC*/, 0);

    ret = mCodecDecoder->init(mIfmtCtx);
    if(!ret){
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
        mIfmtCtx = NULL;
        return false;
    }

    return true;
}

bool VideoInputStream::closeStream() {

    free_input_threads();

    if(mIfmtCtx) {
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
    }

    if(mCodecDecoder)
        delete mCodecDecoder;

    return true;
}

bool VideoInputStream::getOneFrameFromStream(AVFrame *frame) {
    AVPacket *pkt = NULL;
    bool Ret = false;
    AVPacket pkt1;
    static int64_t lastPts = 0;
    pkt = av_packet_alloc();
    if(!pkt){
        qDebug() << "av_packet_alloc fail pkt == NULL.";
        return false;
    }

//    av_init_packet(&pkt1);

    /*int ret = get_input_packet_mt(pkt);
    if (ret < 0) {
        //printf("read error\n");
        //qDebug() << "read error";
        av_packet_free(&pkt);
        //av_packet_unref(&pkt1);
        pkt = NULL;
        return false;
    }*/
    //qDebug() << "VideoInputStream av_read_frame";
    int ret = av_read_frame(mIfmtCtx, pkt);
    if (ret < 0) {
        av_usleep(10000);
        qDebug() << "VideoInputStream read error";
        av_packet_free(&pkt);
        //av_free_packet(&pkt1);
        pkt = NULL;
        return false;
    }
    //qDebug() << "packet pts: " << pkt->pts;
    //qDebug() << "packet lastPts: " << lastPts;
    if(lastPts == pkt->pts){
        av_packet_free(&pkt);
        qDebug() << "VideoInputStream packet pts is same to last pts, drop it";
        return false;
    }
    lastPts = pkt->pts;
    Ret = mCodecDecoder->decode(pkt,frame);
    if(!Ret){
        qDebug() << "VideoInputStream decode fail.";
        av_packet_free(&pkt);
        //av_packet_unref(&pkt1);
        pkt = NULL;
        return false;
    }
    //qDebug() << "VideoInputStream decode success";
    //av_packet_unref(&pkt1);
    av_packet_free(&pkt);
    pkt = NULL;
    return true;
}

bool VideoInputStream::initFifo(AVCodecContext *output_codec_context)
{
    return false;
}

int VideoInputStream::getFifoSize(){
    return 0;
}

bool VideoInputStream::addSampleToFifo(AVFrame *frame){
    return false;
}

bool VideoInputStream::readSampleFromFifo(AVFrame *frame, AVCodecContext *output_codec_context){
    return false;
}

bool VideoInputStream::readSampleFromFifo1(AVFrame **frame, AVCodecContext *output_codec_context){

}

enum AVMediaType VideoInputStream::getCodecMediaType(){
    //printf("VideoInputStream mCodecMediaType: %d\n", mCodecMediaType);
    return mCodecMediaType;
}
