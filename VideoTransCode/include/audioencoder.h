#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <string>
#include <functional>
#include <memory>
#include "queue.h"
#include "thread_manager.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

using namespace std;

using AudioPacketCallback = function<void(AVPacket *)>;

class AudioEncoder
{
public:
    AudioEncoder();
    ~AudioEncoder();
    bool init(int sampleRate, int channels, AVSampleFormat inputFormat,
              const string &codecName = "aac", int bitrate = 128000);

    bool encodeFrame(uint8_t *data, int size, int64_t pts = AV_NOPTS_VALUE);
    bool flush();
    void setPacketCallback(AudioPacketCallback callback);

    AVCodecContext *getCodecContext() const;
    AVCodecParameters *getCodecParameters() const;
    AVRational getTimeBase() const;

private:
    AVCodecContext *codecContext_ = nullptr;
    SwrContext *swrContext_ = nullptr;
    AVSampleFormat inputFormat_ = AV_SAMPLE_FMT_NONE;
    int inputSampleRate_ = 0;
    int inputChannels_ = 0;
    uint64_t inputChannelLayout_ = 0;
    AudioPacketCallback packetCallback_ = nullptr;
    AVFrame *frame_ = nullptr;
    int64_t nextPts_ = 0;
    bool initResampler();
    void processPacket(AVPacket *packet);
};

#endif // AUDIOENCODER_H
