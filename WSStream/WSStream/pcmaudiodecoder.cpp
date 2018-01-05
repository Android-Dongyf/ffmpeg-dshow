#include "pcmaudiodecoder.h"

PcmAudioDecoder::PcmAudioDecoder()
{

}

PcmAudioDecoder::~PcmAudioDecoder(){
    closeDecoder();
}

bool PcmAudioDecoder::init(AVFormatContext *fmtCtx){
    bool ret = openDecoder(fmtCtx);
    if(!ret) {
       printf("openDecoder fail. \n");
    }

    return ret;
}

bool PcmAudioDecoder::decode(AVPacket *pkt, AVFrame *frame){
    int ret, gotFrame = 0;

    ret = avcodec_decode_audio4(mCodeCtx, frame, &gotFrame, pkt);
    if(ret < 0){
        qDebug() << "PcmAudioDecoder decode errer";
        return false;
    }

    if(gotFrame)
        return true;
    else
        return false;
}

bool PcmAudioDecoder::openDecoder(AVFormatContext *fmtCtx){
    if(fmtCtx == NULL)
        return false;

    int audioIndex = -1;
    for(unsigned int i = 0; i < fmtCtx->nb_streams; i++)
        if(fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
               audioIndex = i;
               break;
        }
#if Debug
    printf("audioIndex: %d\n", audioIndex);
#endif
    mCodeCtx = fmtCtx->streams[audioIndex]->codec;
    mCodec = avcodec_find_decoder(mCodeCtx->codec_id);
    if (!mCodec) {
       fprintf(stderr, "Codec not found\n");
       return false;
    }

    /*mParser = av_parser_init(mCodec->id);
    if (!mParser) {
        fprintf(stderr, "parser not found\n");
        return false;
    }*/
/*
    mCodeCtx = avcodec_alloc_context3(mCodec);
    if(!mCodeCtx){
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }

    mCodeCtx->pix_fmt = AV_PIX_FMT_YUYV422;
    mCodeCtx->coded_width = 352;
    mCodeCtx->coded_height = 288;
    AVRational framerate = {25, 1};
    mCodeCtx->framerate = framerate;
*/
    /* open it */
    if (avcodec_open2(mCodeCtx, mCodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        //av_parser_close(mParser);
        return false;
    }
#if Debug
    printf("pcm timebase  Numerator: %d Denominator: %d\n", mCodeCtx->time_base.num, mCodeCtx->time_base.den);
#endif
    return true;
}

bool PcmAudioDecoder::closeDecoder(){

    return true;
}
