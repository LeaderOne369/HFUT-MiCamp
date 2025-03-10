#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#include <functional>
#include <memory>
#include <string>

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}

#include "audio_decoder.h"
#include "ring_buffer.h"

class AudioFilter
{
public:
    using FrameCallback = std::function<void(const AudioFrame *)>;

    AudioFilter();
    ~AudioFilter();

    bool init(int sampleRate, int channels, AVSampleFormat format, const std::string &filterDesc);
    bool processFrame(const AudioFrame *inputFrame);
    void setFrameCallback(FrameCallback callback);
    void reset();

private:
    AVFilterContext *bufferSrcCtx_ = nullptr;
    AVFilterContext *bufferSinkCtx_ = nullptr;
    AVFilterGraph *filterGraph_ = nullptr;

    int sampleRate_ = 0;
    int channels_ = 0;
    AVSampleFormat format_ = AV_SAMPLE_FMT_NONE;
    std::string filterDesc_;

    FrameCallback frameCallback_ = nullptr;

    // 添加环形缓冲区用于音频数据处理
    RingBuffer sampleBuffer_{1024 * 1024};
    int64_t currentSamplePos_ = 0;
    int64_t lastInputPts_ = 0;
    int samplesPerFrame_ = 0;
    double ptsPerSample_ = 0.0;

    bool initFilterGraph();
};

#endif // AUDIO_FILTER_H
