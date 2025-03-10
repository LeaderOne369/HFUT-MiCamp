#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <functional>
#include <memory>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include "video_decoder.h"
#include "queue.h"

struct VideoPacket
{
    AVPacket *packet;
    int64_t pts;
    int64_t dts;
    int streamIndex;

    VideoPacket() : packet(nullptr), pts(0), dts(0), streamIndex(0) {}
    ~VideoPacket()
    {
        if (packet)
        {
            av_packet_free(&packet);
        }
    }
};

class VideoEncoder
{
public:
    using PacketCallback = std::function<void(VideoPacket *)>;

    VideoEncoder();
    ~VideoEncoder();

    bool init(int width, int height, AVPixelFormat pixFmt,
              const std::string &codecName, int bitrate, int fps);
    bool encodeFrame(const VideoFrame *frame);
    void setPacketCallback(PacketCallback callback);
    void flush();
    void reset();

    AVCodecContext *getCodecContext() const { return codecCtx_; }

private:
    AVCodecContext *codecCtx_ = nullptr;
    const AVCodec *codec_ = nullptr;

    int width_ = 0;
    int height_ = 0;
    AVPixelFormat pixFmt_ = AV_PIX_FMT_NONE;
    int bitrate_ = 0;
    int fps_ = 0;

    PacketCallback packetCallback_ = nullptr;

    bool sendPacket(AVPacket *packet);
};

#endif // VIDEO_ENCODER_H