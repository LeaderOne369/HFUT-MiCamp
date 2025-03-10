#include "../include/audio_decoder.h"
#include <iostream>
#include <thread>
#include <chrono>

AudioDecoder::AudioDecoder(shared_ptr<AudioPacketQueue> packetQueue,
                           AVCodecParameters *codecParams,
                           AVRational timeBase)
    : codecContext_(nullptr),
      swrContext_(nullptr),
      packetQueue_(packetQueue),
      outputSampleRate_(48000),
      outputChannels_(2),
      outputFormat_(AV_SAMPLE_FMT_S16)
{

    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec)
    {
        return;
    }

    codecContext_ = avcodec_alloc_context3(codec);
    if (!codecContext_)
    {
        return;
    }

    if (avcodec_parameters_to_context(codecContext_, codecParams) < 0)
    {
        avcodec_free_context(&codecContext_);
        return;
    }

    codecContext_->time_base = timeBase;

    threadManager_ = make_unique<ThreadManager>("AudioDecoder");
    threadManager_->setThreadFunction([this]()
                                      { decodeThreadFunc(); });
}

AudioDecoder::~AudioDecoder()
{
    stop();

    if (swrContext_)
    {
        swr_free(&swrContext_);
    }

    if (codecContext_)
    {
        avcodec_free_context(&codecContext_);
    }
}

bool AudioDecoder::init()
{
    if (!codecContext_)
    {
        return false;
    }

    if (avcodec_open2(codecContext_, codecContext_->codec, nullptr) < 0)
    {
        return false;
    }

    swrContext_ = swr_alloc();
    if (!swrContext_)
    {
        return false;
    }

    uint64_t in_ch_layout = 0;
#if LIBAVUTIL_VERSION_MAJOR >= 57
    AVChannelLayout in_ch_layout_obj;
    av_channel_layout_default(&in_ch_layout_obj, codecContext_->ch_layout.nb_channels);
    in_ch_layout = in_ch_layout_obj.u.mask;

    AVChannelLayout out_ch_layout_obj;
    av_channel_layout_default(&out_ch_layout_obj, outputChannels_);

    swr_alloc_set_opts2(&swrContext_,
                        &out_ch_layout_obj, outputFormat_, outputSampleRate_,
                        &codecContext_->ch_layout, codecContext_->sample_fmt, codecContext_->sample_rate,
                        0, nullptr);
#else
    in_ch_layout = codecContext_->channel_layout;
    if (in_ch_layout == 0)
    {
        in_ch_layout = av_get_default_channel_layout(codecContext_->channels);
    }

    uint64_t out_ch_layout = av_get_default_channel_layout(outputChannels_);

    swrContext_ = swr_alloc_set_opts(swrContext_,
                                     out_ch_layout, outputFormat_, outputSampleRate_,
                                     in_ch_layout, codecContext_->sample_fmt, codecContext_->sample_rate,
                                     0, nullptr);
#endif

    if (swr_init(swrContext_) < 0)
    {
        swr_free(&swrContext_);
        return false;
    }

    return true;
}

void AudioDecoder::start()
{
    threadManager_->start();
}

void AudioDecoder::stop()
{
    threadManager_->stop();
}

void AudioDecoder::setFrameCallback(AudioFrameCallback callback)
{
    frameCallback_ = callback;
}

void AudioDecoder::setOutputFormat(int sampleRate, int channels, AVSampleFormat format)
{
    outputSampleRate_ = sampleRate;
    outputChannels_ = channels;
    outputFormat_ = format;
}

void AudioDecoder::decodeThreadFunc()
{
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    while (threadManager_->isRunning())
    {
        PacketPtr pkt;
        if (!packetQueue_->pop(pkt))
        {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        packet->data = pkt->data;
        packet->size = pkt->size;
        packet->pts = pkt->pts;
        packet->dts = pkt->dts;

        int ret = avcodec_send_packet(codecContext_, packet);
        if (ret < 0)
        {
            continue;
        }

        while (ret >= 0)
        {
            ret = avcodec_receive_frame(codecContext_, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                goto end;
            }

            if (frameCallback_ && swrContext_)
            {
                int outSamples = av_rescale_rnd(
                    swr_get_delay(swrContext_, codecContext_->sample_rate) + frame->nb_samples,
                    outputSampleRate_,
                    codecContext_->sample_rate,
                    AV_ROUND_UP);

                int outSize = av_samples_get_buffer_size(
                    nullptr,
                    outputChannels_,
                    outSamples,
                    outputFormat_,
                    1);

                uint8_t *outBuffer = new uint8_t[outSize];

                uint8_t *outData[1] = {outBuffer};

                int samplesOut = swr_convert(
                    swrContext_,
                    outData,
                    outSamples,
                    (const uint8_t **)frame->data,
                    frame->nb_samples);

                if (samplesOut > 0)
                {
                    int actualSize = av_samples_get_buffer_size(
                        nullptr,
                        outputChannels_,
                        samplesOut,
                        outputFormat_,
                        1);

                    AudioFramePtr audioFrame = make_shared<AudioFrame>();
                    audioFrame->data = outBuffer;
                    audioFrame->size = actualSize;
                    audioFrame->sampleRate = outputSampleRate_;
                    audioFrame->channels = outputChannels_;
                    audioFrame->pts = frame->pts;

                    frameCallback_(audioFrame);
                }
                else
                {
                    delete[] outBuffer;
                }
            }

            av_frame_unref(frame);
        }
    }

    avcodec_send_packet(codecContext_, nullptr);
    while (true)
    {
        int ret = avcodec_receive_frame(codecContext_, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            break;
        }

        if (frameCallback_ && swrContext_)
        {
            int outSamples = av_rescale_rnd(
                swr_get_delay(swrContext_, codecContext_->sample_rate) + frame->nb_samples,
                outputSampleRate_,
                codecContext_->sample_rate,
                AV_ROUND_UP);

            int outSize = av_samples_get_buffer_size(
                nullptr,
                outputChannels_,
                outSamples,
                outputFormat_,
                1);

            uint8_t *outBuffer = new uint8_t[outSize];

            uint8_t *outData[1] = {outBuffer};

            int samplesOut = swr_convert(
                swrContext_,
                outData,
                outSamples,
                (const uint8_t **)frame->data,
                frame->nb_samples);

            if (samplesOut > 0)
            {
                int actualSize = av_samples_get_buffer_size(
                    nullptr,
                    outputChannels_,
                    samplesOut,
                    outputFormat_,
                    1);

                AudioFramePtr audioFrame = make_shared<AudioFrame>();
                audioFrame->data = outBuffer;
                audioFrame->size = actualSize;
                audioFrame->sampleRate = outputSampleRate_;
                audioFrame->channels = outputChannels_;
                audioFrame->pts = frame->pts;

                frameCallback_(audioFrame);
            }
            else
            {
                delete[] outBuffer;
            }
        }

        av_frame_unref(frame);
    }

end:
    av_frame_free(&frame);
    av_packet_free(&packet);
}