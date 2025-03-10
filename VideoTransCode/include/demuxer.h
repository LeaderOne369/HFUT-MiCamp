#ifndef DEMUXER_H
#define DEMUXER_H

#include <string>
#include <fstream>
#include "queue.h"
#include "thread_manager.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

using namespace std;

class Demuxer
{
private:
    string inputFile_;
    AVFormatContext *formatContext_;
    int videoStreamIndex_;
    int audioStreamIndex_;

    shared_ptr<VideoPacketQueue> videoQueue_;
    shared_ptr<AudioPacketQueue> audioQueue_;

    unique_ptr<ThreadManager> threadManager_;

    ofstream logFile_;

    void demuxThreadFunc();

public:
    Demuxer(const string &inputFile,
            shared_ptr<VideoPacketQueue> videoQueue,
            shared_ptr<AudioPacketQueue> audioQueue);
    ~Demuxer();

    bool init();
    void start();
    void stop();

    AVCodecParameters *getVideoCodecParameters();
    AVCodecParameters *getAudioCodecParameters();
    AVRational getVideoTimeBase();
    AVRational getAudioTimeBase();
};

#endif // DEMUXER_H