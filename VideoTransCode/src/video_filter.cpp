#include "video_filter.h"
#include <iostream>

using namespace std;

VideoFilter::VideoFilter() {}

VideoFilter::~VideoFilter()
{
    reset();
}

bool VideoFilter::init(int width, int height, AVPixelFormat format, const std::string &filterDesc)
{
    width_ = width;
    height_ = height;
    format_ = format;
    filterDesc_ = filterDesc;

    return initFilterGraph();
}

bool VideoFilter::initFilterGraph()
{
    char args[512];
    int ret = 0;

    // 创建过滤器图
    filterGraph_ = avfilter_graph_alloc();
    if (!filterGraph_)
    {
        cout << "无法创建过滤器图" << endl;
        return false;
    }

    // 创建输入缓冲区
    const AVFilter *bufferSrc = avfilter_get_by_name("buffer");
    if (!bufferSrc)
    {
        cout << "无法找到buffer过滤器" << endl;
        return false;
    }

    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=1/1000",
             width_, height_, format_);

    ret = avfilter_graph_create_filter(&bufferSrcCtx_, bufferSrc, "in", args, nullptr, filterGraph_);
    if (ret < 0)
    {
        cout << "无法创建buffer源过滤器" << endl;
        return false;
    }

    // 创建输出缓冲区
    const AVFilter *bufferSink = avfilter_get_by_name("buffersink");
    if (!bufferSink)
    {
        cout << "无法找到buffersink过滤器" << endl;
        return false;
    }

    ret = avfilter_graph_create_filter(&bufferSinkCtx_, bufferSink, "out", nullptr, nullptr, filterGraph_);
    if (ret < 0)
    {
        cout << "无法创建buffer接收过滤器" << endl;
        return false;
    }

    // 设置输出像素格式
    enum AVPixelFormat pix_fmts[] = {format_, AV_PIX_FMT_NONE};
    ret = av_opt_set_int_list(bufferSinkCtx_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        cout << "无法设置输出像素格式" << endl;
        return false;
    }

    // 解析过滤器描述
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    if (!outputs || !inputs)
    {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        cout << "无法分配过滤器输入/输出" << endl;
        return false;
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = bufferSrcCtx_;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = bufferSinkCtx_;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    ret = avfilter_graph_parse_ptr(filterGraph_, filterDesc_.c_str(), &inputs, &outputs, nullptr);
    if (ret < 0)
    {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        cout << "无法解析过滤器描述: " << filterDesc_ << endl;
        return false;
    }

    ret = avfilter_graph_config(filterGraph_, nullptr);
    if (ret < 0)
    {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        cout << "无法配置过滤器图" << endl;
        return false;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    cout << "视频过滤器初始化成功" << endl;
    return true;
}

bool VideoFilter::processFrame(const VideoFrame *inputFrame)
{
    if (!filterGraph_ || !bufferSrcCtx_ || !bufferSinkCtx_)
    {
        cout << "过滤器未初始化" << endl;
        return false;
    }

    if (!inputFrame || !inputFrame->frame)
    {
        cout << "输入帧无效" << endl;
        return false;
    }

    int ret = av_buffersrc_add_frame_flags(bufferSrcCtx_, inputFrame->frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0)
    {
        cout << "无法将帧添加到过滤器" << endl;
        return false;
    }

    while (true)
    {
        AVFrame *filteredFrame = av_frame_alloc();
        if (!filteredFrame)
        {
            cout << "无法分配过滤后的帧" << endl;
            return false;
        }

        ret = av_buffersink_get_frame(bufferSinkCtx_, filteredFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_frame_free(&filteredFrame);
            break;
        }

        if (ret < 0)
        {
            av_frame_free(&filteredFrame);
            cout << "从过滤器获取帧失败" << endl;
            return false;
        }

        if (frameCallback_)
        {
            VideoFrame *outputFrame = new VideoFrame();
            outputFrame->frame = filteredFrame;
            outputFrame->pts = filteredFrame->pts;

            frameCallback_(outputFrame);

            // 回调函数负责释放帧
        }
        else
        {
            av_frame_free(&filteredFrame);
        }
    }

    return true;
}

void VideoFilter::setFrameCallback(FrameCallback callback)
{
    frameCallback_ = callback;
}

void VideoFilter::reset()
{
    if (filterGraph_)
    {
        avfilter_graph_free(&filterGraph_);
        filterGraph_ = nullptr;
    }

    bufferSrcCtx_ = nullptr;
    bufferSinkCtx_ = nullptr;

    width_ = 0;
    height_ = 0;
    format_ = AV_PIX_FMT_NONE;
    filterDesc_.clear();
    frameCallback_ = nullptr;
}