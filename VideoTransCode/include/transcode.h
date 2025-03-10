#ifndef TRANSCODE_H
#define TRANSCODE_H

#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include "demuxer.h"
#include "video_decoder.h"
#include "audio_decoder.h"

using namespace std;

class Transcode
{
private:
    string inputFile_;
    string outputVideoFile_;
    string outputAudioFile_;

    shared_ptr<VideoPacketQueue> videoPacketQueue_;
    shared_ptr<AudioPacketQueue> audioPacketQueue_;

    unique_ptr<Demuxer> demuxer_;
    unique_ptr<VideoDecoder> videoDecoder_;
    unique_ptr<AudioDecoder> audioDecoder_;

    ofstream videoOutFile_;
    ofstream audioOutFile_;

    std::mutex videoFileMutex_;
    std::mutex audioFileMutex_;

    ofstream videoLogFile_;
    ofstream audioLogFile_;

    int frameCount_;
    int sampleCount_;

    void onVideoFrame(VideoFramePtr frame);
    void onAudioFrame(AudioFramePtr frame);

    bool saveVideoFrame(VideoFramePtr frame);
    bool saveAudioFrame(AudioFramePtr frame);

public:
    Transcode(const string &inputFile, const string &outputVideoFile, const string &outputAudioFile);
    ~Transcode();

    bool init();
    bool start();
    void stop();
    void waitForCompletion();

    int getFrameCount() const { return frameCount_; }
    int getSampleCount() const { return sampleCount_; }
};

#endif // TRANSCODE_H
