#ifndef VIDEO_FILTER_H
#define VIDEO_FILTER_H

#include <functional>
#include <memory>
#include <string>

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

#include "video_decoder.h"

class VideoFilter
{
public:
    using FrameCallback = std::function<void(const VideoFrame *)>;

    VideoFilter();
    ~VideoFilter();

    bool init(int width, int height, AVPixelFormat format, const std::string &filterDesc);
    bool processFrame(const VideoFrame *inputFrame);
    void setFrameCallback(FrameCallback callback);
    void reset();

private:
    AVFilterContext *bufferSrcCtx_ = nullptr;
    AVFilterContext *bufferSinkCtx_ = nullptr;
    AVFilterGraph *filterGraph_ = nullptr;

    int width_ = 0;
    int height_ = 0;
    AVPixelFormat format_ = AV_PIX_FMT_NONE;
    std::string filterDesc_;

    FrameCallback frameCallback_ = nullptr;

    bool initFilterGraph();
};

#endif