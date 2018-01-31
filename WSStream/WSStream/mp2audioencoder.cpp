#include "mp2audioencoder.h"

Mp2AudioEncoder::Mp2AudioEncoder()
{
    mCodec = NULL;
    mCodecCtx = NULL;
    mSwrAudioCtx = NULL;
}

Mp2AudioEncoder::~Mp2AudioEncoder(){
    closeEncoder();
}

bool Mp2AudioEncoder::init(){
    bool ret = openEncoder();
    if(!ret) {
       printf("openEncoder fail. \n");
    }

    return ret;
}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

bool Mp2AudioEncoder::encode(AVFrame *frame, AVPacket *pkt){
    int ret, got_packet = 0;
#if 0
    if (frame) {
             /* convert samples from native format to destination codec format, using the resampler */
                    /* compute destination number of samples */
            int dst_nb_samples = av_rescale_rnd(swr_get_delay(mSwrAudioCtx, mCodecCtx->sample_rate) + frame->nb_samples,
                                                    mCodecCtx->sample_rate, mCodecCtx->sample_rate, AV_ROUND_UP);
            av_assert0(dst_nb_samples == frame->nb_samples);
            /* when we pass a frame to the encoder, it may keep a reference to it
             * internally;
             * make sure we do not overwrite it here
             */
            int nb_samples = 0;
            if (mCodecCtx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
                    nb_samples = 10000;
            else
                    nb_samples = mCodecCtx->frame_size;
            printf("nb_samples: %d\n", nb_samples);
            AVFrame *tmpFrame = alloc_audio_frame(mCodecCtx->sample_fmt, mCodecCtx->channel_layout,
                                                  mCodecCtx->sample_rate, nb_samples);
            printf("123123123\n");
            ret = av_frame_make_writable(tmpFrame);
            if (ret < 0){
                printf("av_frame_make_writable fail.\n");
                return false;
            }
            printf("12312312wqeqe3\n");
            /* convert to destination format */
            ret = swr_convert(mSwrAudioCtx,
                              tmpFrame->data, dst_nb_samples,
                              (const uint8_t **)frame->data, frame->nb_samples);
            if (ret < 0) {
               fprintf(stderr, "Error while converting\n");
               return false;
            }
            printf("asdfadsad\n");
            *frame = *tmpFrame;
            AVRational tmp = {1, mCodecCtx->sample_rate};
            frame->pts = av_rescale_q(samples_count, tmp, mCodecCtx->time_base);
            samples_count += dst_nb_samples;

            av_frame_free(&tmpFrame);
        }
#endif

    ret = avcodec_encode_audio2(mCodecCtx, pkt, frame, &got_packet);
    if(ret < 0) {
        qDebug() << "avcodec_encode_audio2 fail.";
        return false;
    }

    if(got_packet)
        return true;
    else
        return false;
}

bool Mp2AudioEncoder::openEncoder(){
    const char *codec_name = DEFAULT_AUDIO_ENCODE_NAME;
    int ret;

    /* find the mp2 encoder */
    mCodec = avcodec_find_encoder_by_name(codec_name);
    if (!mCodec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        return false;
    }

    mCodecCtx = avcodec_alloc_context3(mCodec);
    if (!mCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }

    /* put sample parameters */
    mCodecCtx->bit_rate = DEFAULT_AUDIO_ENCODE_BITRATE;
    //mCodecCtx->bits_per_raw_sample
    /* resolution must be a multiple of two */
    mCodecCtx->sample_rate = StreamConfig::audio_sample_rate_val();//DEFAULT_AUDIO_SAMPLE_RATE;
    mCodecCtx->sample_fmt = mCodec->sample_fmts ?  mCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    mCodecCtx->channels = StreamConfig::audio_input_channel_val();//DEFAULT_AUDIO_CHANNELS;
    mCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    printf("fmt: %d\n", mCodecCtx->sample_fmt);
    ret = avcodec_open2(mCodecCtx, mCodec, NULL);
    if (ret < 0) {
        printf("avcodec_open2 error, ret: %d\n", ret);
        avcodec_free_context(&mCodecCtx);

        //char *err = av_err2str(ret);
        //fprintf(stderr, "Could not open codec: %s\n", err);
        return false;
    }

    mSwrAudioCtx = swr_alloc();
    if (!mSwrAudioCtx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        avcodec_free_context(&mCodecCtx);
        return false;
    }

    /* set options */
    av_opt_set_int       (mSwrAudioCtx, "in_channel_count",   mCodecCtx->channels,       0);
    av_opt_set_int       (mSwrAudioCtx, "in_sample_rate",     mCodecCtx->sample_rate,    0);
    av_opt_set_sample_fmt(mSwrAudioCtx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (mSwrAudioCtx, "out_channel_count",  mCodecCtx->channels,       0);
    av_opt_set_int       (mSwrAudioCtx, "out_sample_rate",    mCodecCtx->sample_rate,    0);
    av_opt_set_sample_fmt(mSwrAudioCtx, "out_sample_fmt",     mCodecCtx->sample_fmt,     0);

    /* initialize the resampling context */
    if ((ret = swr_init(mSwrAudioCtx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        avcodec_free_context(&mCodecCtx);
        swr_free(&mSwrAudioCtx);
        return false;
    }

    return true;
}

bool Mp2AudioEncoder::closeEncoder(){
    if(mCodecCtx)
        avcodec_close(mCodecCtx);
    if(mCodecCtx)
        avcodec_free_context(&mCodecCtx);
    if(mSwrAudioCtx)
        swr_free(&mSwrAudioCtx);

    return true;
}

AVCodecContext *Mp2AudioEncoder::getCodecContex(){
    return mCodecCtx;
}
