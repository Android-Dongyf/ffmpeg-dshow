#include "audioinputstream.h"

AudioInputStream::AudioInputStream()
:InputStream()
{

}

AudioInputStream::~AudioInputStream(){
    closeStream();
}

bool AudioInputStream::initFifo(AVCodecContext *output_codec_context)
{
    return initAudioFifo(output_codec_context);
}

int AudioInputStream::getFifoSize()
{
    return av_audio_fifo_size(mAudioFifo);
}

bool AudioInputStream::addSampleToFifo(AVFrame *frame)
{
    int ret;
    if(frame == NULL)
        return false;

    ret = av_audio_fifo_realloc(mAudioFifo, av_audio_fifo_size(mAudioFifo) + frame->nb_samples);
    if(ret < 0){
        printf("av_audio_fifo_realloc fail.\n");
        return false;
    }

    if (av_audio_fifo_write(mAudioFifo, (void **)frame->data,
                                frame->nb_samples) < frame->nb_samples) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return false;
    }

    return true;
}

static int init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size)
{
    int error;

    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        //fprintf(stderr, "Could not allocate output frame samples (error '%s')\n",
          //      av_err2str(error));
        av_frame_free(frame);
        return error;
    }

    return 0;
}

bool AudioInputStream::readSampleFromFifo1(AVFrame **frame, AVCodecContext *output_codec_context){
    const int frame_size = FFMIN(av_audio_fifo_size(mAudioFifo),
                                     output_codec_context->frame_size);

    if (init_output_frame(frame, output_codec_context, frame_size)){
        qDebug() << "init_output_frame fail.";
        *frame = NULL;
        return false;
    }

    if (av_audio_fifo_read(mAudioFifo, (void **)((*frame)->data), frame_size) < frame_size) {
         qDebug() << "Could not read data from FIFO";
        av_frame_free(frame);
        *frame = NULL;
        return false;
    }

    return true;
}

bool AudioInputStream::readSampleFromFifo(AVFrame *frame, AVCodecContext *output_codec_context)
{
    //此函数有内存问题
    AVFrame *output_frame;
    const int frame_size = FFMIN(av_audio_fifo_size(mAudioFifo),
                                     output_codec_context->frame_size);

    if (init_output_frame(&output_frame, output_codec_context, frame_size)){
        qDebug() << "init_output_frame fail.";
        frame = NULL;
        return false;
    }

    if (av_audio_fifo_read(mAudioFifo, (void **)output_frame->data, frame_size) < frame_size) {
         qDebug() << "Could not read data from FIFO";
        av_frame_free(&output_frame);
        frame = NULL;
        return false;
    }

    *frame = *output_frame;

     av_frame_free(&output_frame);
    return true;
}


bool AudioInputStream::initAudioFifo(AVCodecContext *output_codec_context)
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(mAudioFifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
                                      output_codec_context->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return false;
    }
    return true;
}

bool AudioInputStream::init() {
    mCodecDecoder = new PcmAudioDecoder();
    if(!mCodecDecoder)
        return false;

    bool ret = openStream();
    if(!ret)
        return false;

    return true;
}

void AudioInputStream::setDeviceLock(QMutex *lock){
    deviceLock = lock;
}

bool AudioInputStream::openStream(){
    int ret = -1;
    AVDictionary *infmtOpt = NULL;
    int sampleRate = StreamConfig::audio_sample_rate_val();//AUDIO_INPUT_SAMPLE_RATE;
    int sampleChs = StreamConfig::audio_input_channel_val();//AUDIO_INPUT_CHANNEL;
    char desc[128] = {0};
    sprintf(desc, "audio=麦克风阵列 %s", StreamConfig::audio_desc_val().c_str());
    qDebug() << "desc: " << desc;
    int audioIndex = -1;
    unsigned int i = 0;

    mIfmt = av_find_input_format(/*AUDIO_INPUT_FORMAT_DESC*/StreamConfig::audio_input_fmt_val().c_str());
    if(!mIfmt) {
        printf( "av_find_input_format fail.\n");
        return false;
    }

    if(mIfmt->priv_class){

        if(av_opt_find(&mIfmt->priv_class, "sample_rate", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set_int(&infmtOpt, "sample_rate", sampleRate, 0);
        }

        if(av_opt_find(&mIfmt->priv_class, "channels", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set_int(&infmtOpt, "channels", sampleChs, 0);
        }

        if(av_opt_find(&mIfmt->priv_class, "sample_size", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set_int(&infmtOpt, "sample_size", 16, 0);
        }

        if(av_opt_find(&mIfmt->priv_class, "audio_buffer_size", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
            av_dict_set_int(&infmtOpt, "audio_buffer_size", 64, 0);
        }
    }

    mIfmtCtx = avformat_alloc_context();
    if(!mIfmtCtx){
        printf( "avformat_alloc_context fail.\n");
        return false;
    }

    if(av_opt_find(&mIfmtCtx->av_class, "rtbufsize", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ)){
        av_dict_set_int(&infmtOpt, "rtbufsize", 184320000, 0);
    }

    if ((ret = avformat_open_input(&mIfmtCtx, desc/*AUDIO_INPUT_DEVICE_DESC*/, mIfmt, &infmtOpt)) < 0) {
        qDebug() << "avformat_open_input fail.  ret: " << ret;
        avformat_free_context(mIfmtCtx);
        return false;
    }

    if ((ret = avformat_find_stream_info(mIfmtCtx, NULL)) < 0) {
        qDebug() << "Failed to retrieve input stream information";
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
        mIfmtCtx = NULL;
        return false;
    }

    if(mIfmtCtx){
#if Debug
        qDebug() << "nb_streams: " << mIfmtCtx->nb_streams;
#endif
    }
    else
        qDebug() << "mIfmtCtx == NULL";

    for(i = 0; i < mIfmtCtx->nb_streams; i++)
        if(mIfmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audioIndex = i;
            mCodecMediaType = AVMEDIA_TYPE_AUDIO;
            break;
        }

    av_dump_format(mIfmtCtx, 0, desc/*AUDIO_INPUT_DEVICE_DESC*/, 0);

    ret = mCodecDecoder->init(mIfmtCtx);
    if(!ret){
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
        mIfmtCtx = NULL;
        return false;
    }

    return true;
}

bool AudioInputStream::closeStream() {
    if(mIfmtCtx) {
        avformat_close_input(&mIfmtCtx);
        avformat_free_context(mIfmtCtx);
    }

    if(mCodecDecoder)
        delete mCodecDecoder;

    if (mAudioFifo)
            av_audio_fifo_free(mAudioFifo);

    return true;
}

bool AudioInputStream::getOneFrameFromStream(AVFrame *frame) {
    AVPacket *pkt = NULL;
    bool Ret = false;

    pkt = av_packet_alloc();
    if(!pkt){
        //printf("av_packet_alloc fail pkt == NULL. \n");
        qDebug() << "AudioInputStream av_packet_alloc fail pkt == NULL.";
        return false;
    }

    int ret = av_read_frame(mIfmtCtx, pkt);
    if (ret < 0) {
        av_usleep(10000);
        //printf("read error\n");
        qDebug() << "[AudioInputStream::getOneFrameFromStream] read error";
        av_packet_free(&pkt);
        pkt = NULL;
        return false;
    }

#if Debug
    //printf("av_read_frame success. \n");
    printf("size: %d. \n", pkt->size);
#endif
    Ret = mCodecDecoder->decode(pkt,frame);
    if(!Ret){
        //printf("decode fail.\n");
        qDebug() << "[AudioInputStream::getOneFrameFromStream] decode fail.";
        av_packet_free(&pkt);
        pkt = NULL;
        return false;
    }

    av_packet_free(&pkt);
    return true;
}

enum AVMediaType AudioInputStream::getCodecMediaType(){
    return mCodecMediaType;
}
