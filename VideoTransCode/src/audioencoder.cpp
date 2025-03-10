#include "audioencoder.h"
#include <iostream>

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavutil/error.h>
}

using namespace std;

AudioEncoder::AudioEncoder() : nextPts_(0)
{
}

AudioEncoder::~AudioEncoder()
{
    if (codecContext_)
    {
        avcodec_free_context(&codecContext_);
    }

    if (swrContext_)
    {
        swr_free(&swrContext_);
    }

    if (frame_)
    {
        av_frame_free(&frame_);
    }
}

bool AudioEncoder::init(int sampleRate, int channels, AVSampleFormat inputFormat,
                        const string &codecName, int bitrate)
{
    inputSampleRate_ = sampleRate;
    inputChannels_ = channels;
    inputFormat_ = inputFormat;

    if (channels == 1)
    {
        inputChannelLayout_ = AV_CH_LAYOUT_MONO;
    }
    else if (channels == 2)
    {
        inputChannelLayout_ = AV_CH_LAYOUT_STEREO;
    }
    else
    {
        cerr << "不支持的通道数: " << channels << endl;
        return false;
    }

    const AVCodec *codec = avcodec_find_encoder_by_name(codecName.c_str());
    if (!codec)
    {
        cerr << "找不到编码器: " << codecName << endl;
        return false;
    }

    codecContext_ = avcodec_alloc_context3(codec);
    if (!codecContext_)
    {
        cerr << "无法分配编码器上下文" << endl;
        return false;
    }

    codecContext_->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    codecContext_->bit_rate = bitrate;
    codecContext_->sample_rate = sampleRate;

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
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

    av_channel_layout_copy(&codecContext_->ch_layout, &ch_layout);
    av_channel_layout_uninit(&ch_layout);
#else
    codecContext_->channels = channels;
    codecContext_->channel_layout = inputChannelLayout_;
#endif

    codecContext_->time_base = (AVRational){1, sampleRate};

    int ret = avcodec_open2(codecContext_, codec, nullptr);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法打开编码器: " << errbuf << endl;
        return false;
    }

    if (!initResampler())
    {
        cerr << "初始化重采样器失败" << endl;
        return false;
    }

    frame_ = av_frame_alloc();
    if (!frame_)
    {
        cerr << "无法分配帧" << endl;
        return false;
    }

    frame_->format = codecContext_->sample_fmt;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
    av_channel_layout_copy(&frame_->ch_layout, &codecContext_->ch_layout);
#else
    frame_->channels = codecContext_->channels;
    frame_->channel_layout = codecContext_->channel_layout;
#endif
    frame_->sample_rate = codecContext_->sample_rate;
    frame_->nb_samples = codecContext_->frame_size;

    ret = av_frame_get_buffer(frame_, 0);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法分配帧缓冲区: " << errbuf << endl;
        return false;
    }

    cout << "音频编码器初始化成功" << endl;
    cout << "编码器: " << codec->name << endl;
    cout << "采样率: " << codecContext_->sample_rate << " Hz" << endl;
    cout << "通道数: " << channels << endl;
    cout << "比特率: " << codecContext_->bit_rate << " bps" << endl;
    cout << "采样格式: " << av_get_sample_fmt_name(codecContext_->sample_fmt) << endl;
    cout << "帧大小: " << codecContext_->frame_size << " 样本" << endl;

    return true;
}

bool AudioEncoder::initResampler()
{
    swrContext_ = swr_alloc();
    if (!swrContext_)
    {
        cerr << "无法分配重采样上下文" << endl;
        return false;
    }

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
    AVChannelLayout inLayout, outLayout;
    av_channel_layout_uninit(&inLayout);
    av_channel_layout_uninit(&outLayout);

    if (inputChannels_ == 1)
    {
        av_channel_layout_from_mask(&inLayout, AV_CH_LAYOUT_MONO);
    }
    else if (inputChannels_ == 2)
    {
        av_channel_layout_from_mask(&inLayout, AV_CH_LAYOUT_STEREO);
    }
    else
    {
        av_channel_layout_default(&inLayout, inputChannels_);
    }

    av_channel_layout_copy(&outLayout, &codecContext_->ch_layout);

    av_opt_set_chlayout(swrContext_, "in_chlayout", &inLayout, 0);
    av_opt_set_chlayout(swrContext_, "out_chlayout", &outLayout, 0);
    av_opt_set_int(swrContext_, "in_sample_rate", inputSampleRate_, 0);
    av_opt_set_int(swrContext_, "out_sample_rate", codecContext_->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext_, "in_sample_fmt", inputFormat_, 0);
    av_opt_set_sample_fmt(swrContext_, "out_sample_fmt", codecContext_->sample_fmt, 0);

    av_channel_layout_uninit(&inLayout);
    av_channel_layout_uninit(&outLayout);
#else
    av_opt_set_int(swrContext_, "in_channel_layout", inputChannelLayout_, 0);
    av_opt_set_int(swrContext_, "out_channel_layout", codecContext_->channel_layout, 0);
    av_opt_set_int(swrContext_, "in_sample_rate", inputSampleRate_, 0);
    av_opt_set_int(swrContext_, "out_sample_rate", codecContext_->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext_, "in_sample_fmt", inputFormat_, 0);
    av_opt_set_sample_fmt(swrContext_, "out_sample_fmt", codecContext_->sample_fmt, 0);
#endif

    int ret = swr_init(swrContext_);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法初始化重采样上下文: " << errbuf << endl;
        return false;
    }

    return true;
}

bool AudioEncoder::encodeFrame(uint8_t *data, int size, int64_t pts)
{
    if (!codecContext_ || !swrContext_ || !frame_)
    {
        cerr << "编码器未初始化" << endl;
        return false;
    }

    int bytesPerSample = av_get_bytes_per_sample(inputFormat_);
    int numSamples = size / (bytesPerSample * inputChannels_);

    AVFrame *inputFrame = av_frame_alloc();
    if (!inputFrame)
    {
        cerr << "无法分配输入帧" << endl;
        return false;
    }

    inputFrame->format = inputFormat_;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
    AVChannelLayout ch_layout;
    av_channel_layout_uninit(&ch_layout);

    if (inputChannels_ == 1)
    {
        av_channel_layout_from_mask(&ch_layout, AV_CH_LAYOUT_MONO);
    }
    else if (inputChannels_ == 2)
    {
        av_channel_layout_from_mask(&ch_layout, AV_CH_LAYOUT_STEREO);
    }
    else
    {
        av_channel_layout_default(&ch_layout, inputChannels_);
    }

    av_channel_layout_copy(&inputFrame->ch_layout, &ch_layout);
    av_channel_layout_uninit(&ch_layout);
#else
    inputFrame->channels = inputChannels_;
    inputFrame->channel_layout = inputChannelLayout_;
#endif
    inputFrame->sample_rate = inputSampleRate_;
    inputFrame->nb_samples = numSamples;

    if (pts != AV_NOPTS_VALUE)
    {
        inputFrame->pts = pts;
    }
    else
    {
        inputFrame->pts = nextPts_;
        nextPts_ += numSamples;
    }

    int ret = av_frame_get_buffer(inputFrame, 0);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法分配输入帧缓冲区: " << errbuf << endl;
        av_frame_free(&inputFrame);
        return false;
    }

    ret = av_frame_make_writable(inputFrame);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法使帧可写: " << errbuf << endl;
        av_frame_free(&inputFrame);
        return false;
    }

    if (av_sample_fmt_is_planar(inputFormat_))
    {
        int planarSize = numSamples * bytesPerSample;
        for (int ch = 0; ch < inputChannels_; ch++)
        {
            memcpy(inputFrame->data[ch], data + ch * planarSize, planarSize);
        }
    }
    else
    {
        memcpy(inputFrame->data[0], data, size);
    }

    ret = av_frame_make_writable(frame_);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法使编码器帧可写: " << errbuf << endl;
        av_frame_free(&inputFrame);
        return false;
    }

    ret = swr_convert_frame(swrContext_, frame_, inputFrame);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "重采样失败: " << errbuf << endl;
        av_frame_free(&inputFrame);
        return false;
    }

    frame_->pts = inputFrame->pts;

    av_frame_free(&inputFrame);

    ret = avcodec_send_frame(codecContext_, frame_);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "发送帧到编码器失败: " << errbuf << endl;
        return false;
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        cerr << "无法分配数据包" << endl;
        return false;
    }

    ret = avcodec_receive_packet(codecContext_, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        av_packet_free(&packet);
        return true;
    }
    else if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "从编码器接收数据包失败: " << errbuf << endl;
        av_packet_free(&packet);
        return false;
    }

    processPacket(packet);

    return true;
}

bool AudioEncoder::flush()
{
    if (!codecContext_)
    {
        cerr << "编码器未初始化" << endl;
        return false;
    }

    int ret = avcodec_send_frame(codecContext_, nullptr);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "刷新编码器失败: " << errbuf << endl;
        return false;
    }

    while (true)
    {
        AVPacket *packet = av_packet_alloc();
        if (!packet)
        {
            cerr << "无法分配数据包" << endl;
            return false;
        }

        ret = avcodec_receive_packet(codecContext_, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_free(&packet);
            break;
        }
        else if (ret < 0)
        {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            cerr << "从编码器接收数据包失败: " << errbuf << endl;
            av_packet_free(&packet);
            return false;
        }

        processPacket(packet);
    }

    return true;
}

void AudioEncoder::processPacket(AVPacket *packet)
{
    if (packetCallback_)
    {
        packetCallback_(packet);
    }
    else
    {
        av_packet_free(&packet);
    }
}

void AudioEncoder::setPacketCallback(AudioPacketCallback callback)
{
    packetCallback_ = callback;
}

AVCodecContext *AudioEncoder::getCodecContext() const
{
    return codecContext_;
}

AVCodecParameters *AudioEncoder::getCodecParameters() const
{
    if (!codecContext_)
    {
        return nullptr;
    }

    AVCodecParameters *params = avcodec_parameters_alloc();
    if (!params)
    {
        cerr << "无法分配编码器参数" << endl;
        return nullptr;
    }

    int ret = avcodec_parameters_from_context(params, codecContext_);
    if (ret < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法从上下文获取编码器参数: " << errbuf << endl;
        avcodec_parameters_free(&params);
        return nullptr;
    }

    return params;
}

AVRational AudioEncoder::getTimeBase() const
{
    if (!codecContext_)
    {
        return (AVRational){0, 0};
    }

    return codecContext_->time_base;
}
