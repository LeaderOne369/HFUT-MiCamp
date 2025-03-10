#include "../include/transcode.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

using namespace std;

Transcode::Transcode(const string &inputFile, const string &outputVideoFile, const string &outputAudioFile)
    : inputFile_(inputFile),
      outputVideoFile_(outputVideoFile),
      outputAudioFile_(outputAudioFile),
      frameCount_(0),
      sampleCount_(0)
{

    videoPacketQueue_ = make_shared<VideoPacketQueue>();
    audioPacketQueue_ = make_shared<AudioPacketQueue>();

    videoLogFile_.open("../resource/video_frames.txt");
    audioLogFile_.open("../resource/audio_frames.txt");
}

Transcode::~Transcode()
{
    stop();

    if (videoOutFile_.is_open())
    {
        videoOutFile_.close();
    }

    if (audioOutFile_.is_open())
    {
        audioOutFile_.close();
    }

    if (videoLogFile_.is_open())
    {
        videoLogFile_.close();
    }

    if (audioLogFile_.is_open())
    {
        audioLogFile_.close();
    }
}

bool Transcode::init()
{
    videoOutFile_.open(outputVideoFile_, ios::binary);
    if (!videoOutFile_.is_open())
    {
        cerr << "无法打开视频输出文件: " << outputVideoFile_ << endl;
        return false;
    }

    audioOutFile_.open(outputAudioFile_, ios::binary);
    if (!audioOutFile_.is_open())
    {
        cerr << "无法打开音频输出文件: " << outputAudioFile_ << endl;
        return false;
    }

    demuxer_ = make_unique<Demuxer>(inputFile_, videoPacketQueue_, audioPacketQueue_);
    if (!demuxer_->init())
    {
        cerr << "初始化解复用器失败" << endl;
        return false;
    }

    AVCodecParameters *videoCodecParams = demuxer_->getVideoCodecParameters();
    AVCodecParameters *audioCodecParams = demuxer_->getAudioCodecParameters();

    if (videoCodecParams)
    {
        videoDecoder_ = make_unique<VideoDecoder>(
            videoPacketQueue_,
            videoCodecParams,
            demuxer_->getVideoTimeBase());

        if (!videoDecoder_->init())
        {
            cerr << "初始化视频解码器失败" << endl;
            return false;
        }

        videoDecoder_->setFrameCallback([this](VideoFramePtr frame)
                                        { onVideoFrame(frame); });
    }

    if (audioCodecParams)
    {
        audioDecoder_ = make_unique<AudioDecoder>(
            audioPacketQueue_,
            audioCodecParams,
            demuxer_->getAudioTimeBase());

        if (!audioDecoder_->init())
        {
            cerr << "初始化音频解码器失败" << endl;
            return false;
        }

        audioDecoder_->setFrameCallback([this](AudioFramePtr frame)
                                        { onAudioFrame(frame); });
    }

    return true;
}

bool Transcode::start()
{
    if (videoDecoder_)
    {
        videoDecoder_->start();
    }

    if (audioDecoder_)
    {
        audioDecoder_->start();
    }

    demuxer_->start();

    return true;
}

void Transcode::stop()
{
    if (demuxer_)
    {
        demuxer_->stop();
    }

    if (videoDecoder_)
    {
        videoDecoder_->stop();
    }

    if (audioDecoder_)
    {
        audioDecoder_->stop();
    }
}

void Transcode::waitForCompletion()
{
    cout << "等待解码器完成..." << endl;

    if (demuxer_)
    {
        demuxer_->stop();
    }

    cout << "等待队列清空..." << endl;
    while (videoPacketQueue_->size() > 0 || audioPacketQueue_->size() > 0)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    if (videoDecoder_)
    {
        cout << "等待视频解码器完成..." << endl;
        this_thread::sleep_for(chrono::seconds(5));
        videoDecoder_->stop();
    }

    if (audioDecoder_)
    {
        cout << "等待音频解码器完成..." << endl;
        this_thread::sleep_for(chrono::seconds(3));
        audioDecoder_->stop();
    }

    if (videoOutFile_.is_open())
    {
        videoOutFile_.flush();
    }

    if (audioOutFile_.is_open())
    {
        audioOutFile_.flush();
    }

    cout << "所有解码器已完成" << endl;
}

void Transcode::onVideoFrame(VideoFramePtr frame)
{
    if (saveVideoFrame(frame))
    {
        frameCount_++;
    }

    if (videoLogFile_.is_open())
    {
        videoLogFile_ << "视频帧: PTS=" << frame->pts
                      << ", 分辨率=" << frame->frame->width << "x" << frame->frame->height
                      << ", 格式=" << frame->frame->format << endl;
    }
}

void Transcode::onAudioFrame(AudioFramePtr frame)
{
    if (saveAudioFrame(frame))
    {
        sampleCount_++;
    }

    if (audioLogFile_.is_open())
    {
        audioLogFile_ << "音频帧: PTS=" << frame->pts
                      << ", 采样率=" << frame->sampleRate
                      << ", 通道数=" << frame->channels
                      << ", 大小=" << frame->size << endl;
    }
}

bool Transcode::saveVideoFrame(VideoFramePtr frame)
{
    if (!frame || !frame->frame || !videoOutFile_.is_open())
    {
        return false;
    }

    AVFrame *avFrame = frame->frame;
    std::lock_guard<std::mutex> lock(videoFileMutex_);

    for (int i = 0; i < avFrame->height; i++)
    {
        videoOutFile_.write(reinterpret_cast<char *>(avFrame->data[0] + i * avFrame->linesize[0]), avFrame->width);
        if (!videoOutFile_.good())
        {
            std::cerr << "写入 Y 平面失败，行: " << i << std::endl;
            return false;
        }
    }

    for (int i = 0; i < avFrame->height / 2; i++)
    {
        videoOutFile_.write(reinterpret_cast<char *>(avFrame->data[1] + i * avFrame->linesize[1]), avFrame->width / 2);
        if (!videoOutFile_.good())
        {
            std::cerr << "写入 U 平面失败，行: " << i << std::endl;
            return false;
        }
    }

    for (int i = 0; i < avFrame->height / 2; i++)
    {
        videoOutFile_.write(reinterpret_cast<char *>(avFrame->data[2] + i * avFrame->linesize[2]), avFrame->width / 2);
        if (!videoOutFile_.good())
        {
            std::cerr << "写入 V 平面失败，行: " << i << std::endl;
            return false;
        }
    }

    videoOutFile_.flush();

    return true;
}

bool Transcode::saveAudioFrame(AudioFramePtr frame)
{
    if (!frame || !frame->data || !audioOutFile_.is_open())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(audioFileMutex_);

    if (frame->size > 0)
    {
        audioOutFile_.write(reinterpret_cast<char *>(frame->data), frame->size);
        if (!audioOutFile_.good())
        {
            std::cerr << "写入音频数据失败，大小: " << frame->size << std::endl;
            return false;
        }
        audioOutFile_.flush();
    }

    return true;
}
