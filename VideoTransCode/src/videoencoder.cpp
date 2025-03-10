#include "videoencoder.h"
#include <iostream>

using namespace std;

VideoEncoder::VideoEncoder() {}

VideoEncoder::~VideoEncoder()
{
    reset();
}

bool VideoEncoder::init(int width, int height, AVPixelFormat pixFmt,
                        const std::string &codecName, int bitrate, int fps)
{
    width_ = width;
    height_ = height;
    pixFmt_ = pixFmt;
    bitrate_ = bitrate;
    fps_ = fps;

    codec_ = avcodec_find_encoder_by_name(codecName.c_str());
    if (!codec_)
    {
        cout << "找不到编码器: " << codecName << endl;
        return false;
    }

    codecCtx_ = avcodec_alloc_context3(codec_);
    if (!codecCtx_)
    {
        cout << "无法分配编码器上下文" << endl;
        return false;
    }

    codecCtx_->width = width_;
    codecCtx_->height = height_;
    codecCtx_->pix_fmt = pixFmt_;
    codecCtx_->bit_rate = bitrate_;
    codecCtx_->time_base = (AVRational){1, fps_};
    codecCtx_->framerate = (AVRational){fps_, 1};
    codecCtx_->gop_size = 12;
    codecCtx_->max_b_frames = 2;

    if (codecName == "libx264")
    {
        av_opt_set(codecCtx_->priv_data, "preset", "medium", 0);
        av_opt_set(codecCtx_->priv_data, "tune", "zerolatency", 0);
    }

    int ret = avcodec_open2(codecCtx_, codec_, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "无法打开编码器: " << errBuf << endl;
        return false;
    }

    cout << "视频编码器初始化成功: " << codecName << ", "
         << width_ << "x" << height_ << ", " << bitrate_ << "bps, " << fps_ << "fps" << endl;
    return true;
}

bool VideoEncoder::encodeFrame(const VideoFrame *frame)
{
    if (!codecCtx_)
    {
        cout << "编码器未初始化" << endl;
        return false;
    }

    int ret = avcodec_send_frame(codecCtx_, frame ? frame->frame : nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "发送帧到编码器失败: " << errBuf << endl;
        return false;
    }

    while (ret >= 0)
    {
        AVPacket *packet = av_packet_alloc();
        if (!packet)
        {
            cout << "无法分配数据包" << endl;
            return false;
        }

        ret = avcodec_receive_packet(codecCtx_, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            av_packet_free(&packet);
            break;
        }
        else if (ret < 0)
        {
            av_packet_free(&packet);
            char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
            cout << "从编码器接收数据包失败: " << errBuf << endl;
            return false;
        }

        if (packetCallback_)
        {
            VideoPacket *videoPacket = new VideoPacket();
            videoPacket->packet = packet;
            videoPacket->pts = packet->pts;
            videoPacket->dts = packet->dts;
            videoPacket->streamIndex = 0;

            packetCallback_(videoPacket);
        }
        else
        {
            av_packet_free(&packet);
        }
    }

    return true;
}

void VideoEncoder::setPacketCallback(PacketCallback callback)
{
    packetCallback_ = callback;
}

void VideoEncoder::flush()
{
    if (codecCtx_)
    {
        encodeFrame(nullptr);
    }
}

void VideoEncoder::reset()
{
    if (codecCtx_)
    {
        avcodec_free_context(&codecCtx_);
        codecCtx_ = nullptr;
    }

    codec_ = nullptr;
    width_ = 0;
    height_ = 0;
    pixFmt_ = AV_PIX_FMT_NONE;
    bitrate_ = 0;
    fps_ = 0;
    packetCallback_ = nullptr;
}

bool VideoEncoder::sendPacket(AVPacket *packet)
{
    if (!packetCallback_ || !packet)
    {
        return false;
    }

    VideoPacket *videoPacket = new VideoPacket();
    videoPacket->packet = packet;
    videoPacket->pts = packet->pts;
    videoPacket->dts = packet->dts;
    videoPacket->streamIndex = 0;

    packetCallback_(videoPacket);
    return true;
}
