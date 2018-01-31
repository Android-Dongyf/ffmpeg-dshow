#ifndef FFMPEGALL_H
#define FFMPEGALL_H
#if _MSC_VER
#define snprintf _snprintf
#endif
extern "C"
{
    #define __STDC_FORMAT_MACROS

    #include <libavcodec/avcodec.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <libavutil/time.h>
    #include <libavutil/avassert.h>
    #include "libavutil/threadmessage.h"
    #include "libavutil/audio_fifo.h"
    #include "libavutil/timestamp.h"
}
#include "common.h"
#endif // FFMPEGALL_H
