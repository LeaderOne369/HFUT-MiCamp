#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <functional>
#include "queue.h"
#include "thread_manager.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

using namespace std;

struct VideoFrame
{
    AVFrame *frame;
    int64_t pts;

    VideoFrame() : frame(nullptr), pts(0) {}
    ~VideoFrame()
    {
        if (frame)
        {
            av_frame_free(&frame);
        }
    }
};

using VideoFramePtr = shared_ptr<VideoFrame>;

using VideoFrameCallback = function<void(VideoFramePtr)>;

class VideoDecoder
{
private:
    AVCodecContext *codecContext_;
    shared_ptr<VideoPacketQueue> packetQueue_;

    unique_ptr<ThreadManager> threadManager_;

    VideoFrameCallback frameCallback_;

    void decodeThreadFunc();

public:
    VideoDecoder(shared_ptr<VideoPacketQueue> packetQueue,
                 AVCodecParameters *codecParams,
                 AVRational timeBase);
    ~VideoDecoder();

    bool init();
    void start();
    void stop();

    void setFrameCallback(VideoFrameCallback callback);
};

#endif // VIDEO_DECODER_H
