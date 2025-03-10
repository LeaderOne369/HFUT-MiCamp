#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <functional>
#include "queue.h"
#include "thread_manager.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

using namespace std;

struct AudioFrame
{
    uint8_t *data;
    int size;
    int sampleRate;
    int channels;
    int64_t pts;

    AudioFrame() : data(nullptr), size(0), sampleRate(0), channels(0), pts(0) {}
    ~AudioFrame()
    {
        if (data)
        {
            delete[] data;
            data = nullptr;
        }
    }
};

using AudioFramePtr = shared_ptr<AudioFrame>;

using AudioFrameCallback = function<void(AudioFramePtr)>;

class AudioDecoder
{
private:
    AVCodecContext *codecContext_;
    SwrContext *swrContext_;
    shared_ptr<AudioPacketQueue> packetQueue_;

    unique_ptr<ThreadManager> threadManager_;

    AudioFrameCallback frameCallback_;

    int outputSampleRate_;
    int outputChannels_;
    AVSampleFormat outputFormat_;

    void decodeThreadFunc();

public:
    AudioDecoder(shared_ptr<AudioPacketQueue> packetQueue,
                 AVCodecParameters *codecParams,
                 AVRational timeBase);
    ~AudioDecoder();

    bool init();
    void start();
    void stop();

    void setFrameCallback(AudioFrameCallback callback);
    void setOutputFormat(int sampleRate, int channels, AVSampleFormat format);
};

#endif // AUDIO_DECODER_H