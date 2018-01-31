#include "videocropfilter.h"

VideoCropFilter::VideoCropFilter(QObject *parent) : QObject(parent)
{
    buffersinkCtx = NULL;;
    buffersrcCtx = NULL;
    filterGraph = NULL;
}

VideoCropFilter::~VideoCropFilter()
{
    if(filterGraph)
        avfilter_graph_free(&filterGraph);

    buffersinkCtx = NULL;;
    buffersrcCtx = NULL;
    filterGraph = NULL;
}

int VideoCropFilter::initFilters(const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

    if(iFmtCtx == NULL || decCtx == NULL)
        return AVERROR(EFAULT);
    time_base = iFmtCtx->streams[0]->time_base;

    filterGraph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filterGraph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            decCtx->width, decCtx->height, decCtx->pix_fmt,
            time_base.num, /*time_base.den*/25,
            /*decCtx->sample_aspect_ratio.num*/1, decCtx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrcCtx, buffersrc, "in",
                                       args, NULL, filterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersinkCtx, buffersink, "out",
                                       NULL, NULL, filterGraph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersinkCtx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrcCtx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersinkCtx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filterGraph, filters_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filterGraph, NULL)) < 0)
        goto end;
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

bool VideoCropFilter::initWithInputFmtCtxAndDecCtx(AVFormatContext *ctx, AVCodecContext *dCtx){
    if(ctx == NULL || dCtx == NULL)
        return false;

    iFmtCtx = ctx;
    decCtx = dCtx;

    //const char *filter_descr = FILTER_DESC;
    int ret = initFilters(StreamConfig::filter_description_val().c_str());
    if(ret < 0){
        qDebug() << "initFilters fail.";
        return false;
    }

    return true;
}

bool VideoCropFilter::cropVideoWithFrame(AVFrame *srcFrame, AVFrame **dstFrame){
    int ret = -1;

    if(srcFrame == NULL || dstFrame == NULL || *dstFrame == NULL){
        qDebug() << "srcFrame == NULL || dstFrame == NULL || *dstFrame == NULL";
        return false;
    }
    qDebug() << "w: " << srcFrame->width << "h: " << srcFrame->height << " fmt: " << srcFrame->format;
    srcFrame->pts = srcFrame->best_effort_timestamp;

    /* push the decoded frame into the filtergraph */
    ret = av_buffersrc_add_frame_flags(buffersrcCtx, srcFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
    //ret = av_buffersrc_add_frame(buffersrcCtx, srcFrame);
    qDebug() << "ret: " << ret;
    if (ret < 0) {
        //av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        qDebug() << "Error while feeding the filtergraph";
        return false;
    }

    /* pull filtered frames from the filtergraph */
    while (1) {
        ret = av_buffersink_get_frame(buffersinkCtx, *dstFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            break;
        }
        if (ret < 0) {
            qDebug() << "av_buffersink_get_frame ";
            return false;
        }
    }

    return true;
}
