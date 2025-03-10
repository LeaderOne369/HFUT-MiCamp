#include "audio_filter.h"
#include <iostream>

using namespace std;

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
static inline void set_frame_channels(AVFrame *frame, int channels)
{
    AVChannelLayout ch_layout;
    av_channel_layout_uninit(&ch_layout);

    if (channels == 1)
    {
        av_channel_layout_from_mask(&ch_layout, AV_CH_LAYOUT_MONO);
    }
    else if (channels == 2)
    {
        av_channel_layout_from_mask(&ch_layout, AV_CH_LAYOUT_STEREO);
    }
    else
    {
        av_channel_layout_default(&ch_layout, channels);
    }

    av_channel_layout_copy(&frame->ch_layout, &ch_layout);
    av_channel_layout_uninit(&ch_layout);
}

static inline int get_frame_channels(const AVFrame *frame)
{
    return frame->ch_layout.nb_channels;
}
#else
static inline void set_frame_channels(AVFrame *frame, int channels)
{
    if (channels == 1)
    {
        frame->channel_layout = AV_CH_LAYOUT_MONO;
    }
    else if (channels == 2)
    {
        frame->channel_layout = AV_CH_LAYOUT_STEREO;
    }
    else
    {
        frame->channel_layout = av_get_default_channel_layout(channels);
    }
    frame->channels = channels;
}

static inline int get_frame_channels(const AVFrame *frame)
{
    return frame->channels;
}
#endif

AudioFilter::AudioFilter() {}

AudioFilter::~AudioFilter()
{
    reset();
}

bool AudioFilter::init(int sampleRate, int channels, AVSampleFormat format, const std::string &filterDesc)
{
    sampleRate_ = sampleRate;
    channels_ = channels;
    format_ = format;
    filterDesc_ = filterDesc;

    // 初始化PTS跟踪变量
    currentSamplePos_ = 0;
    lastInputPts_ = 0;
    samplesPerFrame_ = 1024;
    ptsPerSample_ = 1.0;

    return initFilterGraph();
}

bool AudioFilter::initFilterGraph()
{
    char args[512];
    int ret = 0;

    filterGraph_ = avfilter_graph_alloc();
    if (!filterGraph_)
    {
        cout << "无法创建音频过滤器图" << endl;
        return false;
    }

    const AVFilter *bufferSrc = avfilter_get_by_name("abuffer");
    if (!bufferSrc)
    {
        cout << "无法找到abuffer过滤器" << endl;
        return false;
    }

    uint64_t channel_layout = 0;
    if (channels_ == 1)
    {
        channel_layout = AV_CH_LAYOUT_MONO;
    }
    else if (channels_ == 2)
    {
        channel_layout = AV_CH_LAYOUT_STEREO;
    }
    else
    {
        cout << "不支持的通道数: " << channels_ << endl;
        return false;
    }

    snprintf(args, sizeof(args),
             "sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64 ":time_base=1/1000",
             sampleRate_, av_get_sample_fmt_name(format_), channel_layout);

    ret = avfilter_graph_create_filter(&bufferSrcCtx_, bufferSrc, "in", args, nullptr, filterGraph_);
    if (ret < 0)
    {
        cout << "无法创建音频buffer源过滤器" << endl;
        return false;
    }

    const AVFilter *bufferSink = avfilter_get_by_name("abuffersink");
    if (!bufferSink)
    {
        cout << "无法找到abuffersink过滤器" << endl;
        return false;
    }

    ret = avfilter_graph_create_filter(&bufferSinkCtx_, bufferSink, "out", nullptr, nullptr, filterGraph_);
    if (ret < 0)
    {
        cout << "无法创建音频buffer接收过滤器" << endl;
        return false;
    }

    enum AVSampleFormat sample_fmts[] = {format_, AV_SAMPLE_FMT_NONE};
    ret = av_opt_set_int_list(bufferSinkCtx_, "sample_fmts", sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        cout << "无法设置输出采样格式" << endl;
        return false;
    }

    int sample_rates[] = {sampleRate_, 0};
    ret = av_opt_set_int_list(bufferSinkCtx_, "sample_rates", sample_rates, 0, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        cout << "无法设置输出采样率" << endl;
        return false;
    }

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
        cout << "无法解析音频过滤器描述: " << filterDesc_ << endl;
        return false;
    }

    ret = avfilter_graph_config(filterGraph_, nullptr);
    if (ret < 0)
    {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        cout << "无法配置音频过滤器图" << endl;
        return false;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    cout << "音频过滤器初始化成功" << endl;
    return true;
}

bool AudioFilter::processFrame(const AudioFrame *inputFrame)
{
    if (!filterGraph_ || !bufferSrcCtx_ || !bufferSinkCtx_)
    {
        cout << "音频过滤器未初始化" << endl;
        return false;
    }

    if (!inputFrame || !inputFrame->data || inputFrame->size <= 0)
    {
        cout << "输入音频帧无效" << endl;
        return false;
    }

    int bytesPerSample = av_get_bytes_per_sample(format_);
    int bytesPerFrame = bytesPerSample * channels_;
    int numSamplesInFrame = inputFrame->size / bytesPerFrame;

    if (samplesPerFrame_ != numSamplesInFrame && numSamplesInFrame > 0)
    {
        samplesPerFrame_ = numSamplesInFrame;
    }

    if (inputFrame->pts > lastInputPts_ && lastInputPts_ != 0)
    {
        int64_t ptsDiff = inputFrame->pts - lastInputPts_;
        ptsPerSample_ = static_cast<double>(ptsDiff) / samplesPerFrame_;
    }
    lastInputPts_ = inputFrame->pts;

    // 将输入帧数据写入环形缓冲区
    size_t bytesWritten = sampleBuffer_.write(inputFrame->data, inputFrame->size);
    if (bytesWritten < inputFrame->size)
    {
        cout << "警告：环形缓冲区已满，部分数据未写入" << endl;
    }

    // 从环形缓冲区读取数据进行处理
    // 使用较小的块大小处理数据，以平滑CPU使用
    const size_t blockSize = 4096;
    uint8_t *blockBuffer = new uint8_t[blockSize];

    size_t alignedBlockSize = (blockSize / bytesPerFrame) * bytesPerFrame;
    if (alignedBlockSize == 0 && bytesPerFrame > 0)
    {
        alignedBlockSize = bytesPerFrame;
    }

    while (sampleBuffer_.dataSize() >= alignedBlockSize)
    {
        size_t bytesRead = sampleBuffer_.read(blockBuffer, alignedBlockSize);
        if (bytesRead == 0)
        {
            break;
        }

        int numSamplesInBlock = bytesRead / bytesPerFrame;

        // 创建临时音频帧用于处理这块数据
        AVFrame *frame = av_frame_alloc();
        if (!frame)
        {
            cout << "无法分配音频帧" << endl;
            delete[] blockBuffer;
            return false;
        }

        frame->sample_rate = inputFrame->sampleRate;
        frame->format = format_;

        frame->pts = inputFrame->pts + static_cast<int64_t>(currentSamplePos_ * ptsPerSample_);

        currentSamplePos_ += numSamplesInBlock;

        set_frame_channels(frame, inputFrame->channels);

        frame->nb_samples = numSamplesInBlock;

        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)
        {
            cout << "无法为音频帧分配缓冲区" << endl;
            av_frame_free(&frame);
            delete[] blockBuffer;
            return false;
        }

        ret = av_samples_fill_arrays(frame->data, frame->linesize,
                                     blockBuffer,
                                     inputFrame->channels,
                                     frame->nb_samples,
                                     (AVSampleFormat)frame->format, 0);
        if (ret < 0)
        {
            cout << "无法填充音频数据" << endl;
            av_frame_free(&frame);
            delete[] blockBuffer;
            return false;
        }

        ret = av_buffersrc_add_frame_flags(bufferSrcCtx_, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
        av_frame_free(&frame);

        if (ret < 0)
        {
            cout << "无法将音频帧添加到过滤器" << endl;
            delete[] blockBuffer;
            return false;
        }
    }

    delete[] blockBuffer;

    // 处理过滤后的帧
    while (true)
    {
        AVFrame *filteredFrame = av_frame_alloc();
        if (!filteredFrame)
        {
            cout << "无法分配过滤后的音频帧" << endl;
            return false;
        }

        int ret = av_buffersink_get_frame(bufferSinkCtx_, filteredFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_frame_free(&filteredFrame);
            break;
        }

        if (ret < 0)
        {
            av_frame_free(&filteredFrame);
            cout << "从音频过滤器获取帧失败" << endl;
            return false;
        }

        if (frameCallback_)
        {
            AudioFrame *outputFrame = new AudioFrame();
            outputFrame->sampleRate = filteredFrame->sample_rate;
            outputFrame->channels = get_frame_channels(filteredFrame);
            outputFrame->pts = filteredFrame->pts;

            int dataSize = av_samples_get_buffer_size(nullptr,
                                                      get_frame_channels(filteredFrame),
                                                      filteredFrame->nb_samples,
                                                      (AVSampleFormat)filteredFrame->format,
                                                      1);

            outputFrame->size = dataSize;
            outputFrame->data = new uint8_t[dataSize];

            if (av_sample_fmt_is_planar((AVSampleFormat)filteredFrame->format))
            {
                int bytesPerSample = av_get_bytes_per_sample((AVSampleFormat)filteredFrame->format);
                int sampleSize = bytesPerSample * get_frame_channels(filteredFrame);
                int planeSize = bytesPerSample * filteredFrame->nb_samples;

                for (int i = 0; i < filteredFrame->nb_samples; i++)
                {
                    for (int ch = 0; ch < get_frame_channels(filteredFrame); ch++)
                    {
                        memcpy(outputFrame->data + i * sampleSize + ch * bytesPerSample,
                               filteredFrame->data[ch] + i * bytesPerSample,
                               bytesPerSample);
                    }
                }
            }
            else
            {
                memcpy(outputFrame->data, filteredFrame->data[0], dataSize);
            }

            frameCallback_(outputFrame);
        }

        av_frame_free(&filteredFrame);
    }

    return true;
}

void AudioFilter::setFrameCallback(FrameCallback callback)
{
    frameCallback_ = callback;
}

void AudioFilter::reset()
{
    if (filterGraph_)
    {
        avfilter_graph_free(&filterGraph_);
        filterGraph_ = nullptr;
    }

    bufferSrcCtx_ = nullptr;
    bufferSinkCtx_ = nullptr;

    sampleRate_ = 0;
    channels_ = 0;
    format_ = AV_SAMPLE_FMT_NONE;
    filterDesc_.clear();
    frameCallback_ = nullptr;

    // 清空环形缓冲区
    sampleBuffer_.clear();

    // 重置PTS跟踪变量
    currentSamplePos_ = 0;
    lastInputPts_ = 0;
    samplesPerFrame_ = 0;
    ptsPerSample_ = 0.0;
}
