#include "../include/video_decoder.h"
#include <iostream>
#include <thread>
#include <chrono>

VideoDecoder::VideoDecoder(shared_ptr<VideoPacketQueue> packetQueue,
                           AVCodecParameters *codecParams,
                           AVRational timeBase)
    : codecContext_(nullptr),
      packetQueue_(packetQueue)
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

    threadManager_ = make_unique<ThreadManager>("VideoDecoder");
    threadManager_->setThreadFunction([this]()
                                      { decodeThreadFunc(); });
}

VideoDecoder::~VideoDecoder()
{
    stop();

    if (codecContext_)
    {
        avcodec_free_context(&codecContext_);
    }
}

bool VideoDecoder::init()
{
    if (!codecContext_)
    {
        return false;
    }

    if (avcodec_open2(codecContext_, codecContext_->codec, nullptr) < 0)
    {
        return false;
    }

    return true;
}

void VideoDecoder::start()
{
    threadManager_->start();
}

void VideoDecoder::stop()
{
    threadManager_->stop();
}

void VideoDecoder::setFrameCallback(VideoFrameCallback callback)
{
    frameCallback_ = callback;
}

void VideoDecoder::decodeThreadFunc()
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

            if (frameCallback_)
            {
                VideoFramePtr videoFrame = make_shared<VideoFrame>();
                videoFrame->frame = av_frame_clone(frame);
                videoFrame->pts = frame->pts;

                frameCallback_(videoFrame);
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

        if (frameCallback_)
        {
            VideoFramePtr videoFrame = make_shared<VideoFrame>();
            videoFrame->frame = av_frame_clone(frame);
            videoFrame->pts = frame->pts;

            frameCallback_(videoFrame);
        }

        av_frame_unref(frame);
    }

end:
    av_frame_free(&frame);
    av_packet_free(&packet);
}