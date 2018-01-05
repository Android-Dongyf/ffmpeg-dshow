#include "ffmpegmanager.h"

FfmpegManager::FfmpegManager(QObject *parent) : QObject(parent)
{
    init();
}

void FfmpegManager::init() {
    qDebug() << avcodec_configuration();

#if DEBUG_FFMPEG
    av_log_set_level(56);
#endif
    avcodec_register_all();
    avdevice_register_all();
    avfilter_register_all();
    av_register_all();
    avformat_network_init();

}

