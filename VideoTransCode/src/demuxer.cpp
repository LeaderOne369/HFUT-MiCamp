#include "../include/demuxer.h"
#include <iostream>
#include <cstring>

using namespace std;

Demuxer::Demuxer(const string &inputFile,
                 shared_ptr<VideoPacketQueue> videoQueue,
                 shared_ptr<AudioPacketQueue> audioQueue)
    : inputFile_(inputFile),
      formatContext_(nullptr),
      videoStreamIndex_(-1),
      audioStreamIndex_(-1),
      videoQueue_(videoQueue),
      audioQueue_(audioQueue)
{
    threadManager_ = make_unique<ThreadManager>("Demuxer");
    threadManager_->setThreadFunction([this]()
                                      { demuxThreadFunc(); });

    logFile_.open("../resource/demuxer_log.txt");
}

Demuxer::~Demuxer()
{
    stop();

    if (formatContext_)
    {
        avformat_close_input(&formatContext_);
    }

    if (logFile_.is_open())
    {
        logFile_.close();
    }
}

bool Demuxer::init()
{
    if (logFile_.is_open())
    {
        logFile_ << "Demuxer::init() - 开始初始化解复用器" << endl;
        logFile_ << "输入文件: " << inputFile_ << endl;
    }

    formatContext_ = avformat_alloc_context();
    if (!formatContext_)
    {
        cerr << "无法分配AVFormatContext" << endl;
        return false;
    }

    int ret = avformat_open_input(&formatContext_, inputFile_.c_str(), nullptr, nullptr);
    if (ret != 0)
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法打开输入文件: " << errbuf << endl;
        return false;
    }

    ret = avformat_find_stream_info(formatContext_, nullptr);
    if (ret < 0)
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        cerr << "无法获取流信息: " << errbuf << endl;
        return false;
    }

    for (unsigned int i = 0; i < formatContext_->nb_streams; i++)
    {
        if (formatContext_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIndex_ < 0)
        {
            videoStreamIndex_ = i;
            if (logFile_.is_open())
            {
                logFile_ << "找到视频流，索引: " << i << endl;
            }
        }
        else if (formatContext_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStreamIndex_ < 0)
        {
            audioStreamIndex_ = i;
            if (logFile_.is_open())
            {
                logFile_ << "找到音频流，索引: " << i << endl;
            }
        }
    }

    if (videoStreamIndex_ < 0 && audioStreamIndex_ < 0)
    {
        cerr << "未找到视频或音频流" << endl;
        return false;
    }

    if (logFile_.is_open())
    {
        logFile_ << "Demuxer::init() - 初始化成功" << endl;
    }
    return true;
}

void Demuxer::start()
{
    threadManager_->start();
}

void Demuxer::stop()
{
    threadManager_->stop();
}

void Demuxer::demuxThreadFunc()
{
    AVPacket *packet = av_packet_alloc();

    if (logFile_.is_open())
    {
        logFile_ << "开始解复用线程..." << endl;
    }
    int packetCount = 0;

    while (threadManager_->isRunning())
    {
        int ret = av_read_frame(formatContext_, packet);
        if (ret < 0)
        {
            char errbuf[256];
            av_strerror(ret, errbuf, sizeof(errbuf));
            if (logFile_.is_open())
            {
                logFile_ << "读取帧结束或错误: " << errbuf << endl;
            }
            else
            {
                cerr << "读取帧结束或错误: " << errbuf << endl;
            }
            break;
        }

        packetCount++;

        if (packet->stream_index == videoStreamIndex_ && videoQueue_)
        {
            PacketPtr pkt = make_shared<Packet>();
            pkt->size = packet->size;
            pkt->data = new uint8_t[packet->size];
            memcpy(pkt->data, packet->data, packet->size);

            pkt->pts = packet->pts;
            pkt->dts = packet->dts;
            pkt->isKeyFrame = (packet->flags & AV_PKT_FLAG_KEY);

            videoQueue_->push(pkt);

            if (packetCount % 100 == 0)
            {
                if (logFile_.is_open())
                {
                    logFile_ << "已处理 " << packetCount << " 个包，其中包含视频包" << endl;
                }
                else
                {
                    cout << "已处理 " << packetCount << " 个包，其中包含视频包" << endl;
                }
            }
        }
        else if (packet->stream_index == audioStreamIndex_ && audioQueue_)
        {
            PacketPtr pkt = make_shared<Packet>();
            pkt->size = packet->size;
            pkt->data = new uint8_t[packet->size];
            memcpy(pkt->data, packet->data, packet->size);

            pkt->pts = packet->pts;
            pkt->dts = packet->dts;

            audioQueue_->push(pkt);
        }

        av_packet_unref(packet);
    }

    if (logFile_.is_open())
    {
        logFile_ << "解复用线程结束，共处理 " << packetCount << " 个包" << endl;
    }
    else
    {
        cout << "解复用线程结束，共处理 " << packetCount << " 个包" << endl;
    }

    av_packet_free(&packet);

    if (videoQueue_)
        videoQueue_->setQuit();
    if (audioQueue_)
        audioQueue_->setQuit();
}

AVCodecParameters *Demuxer::getVideoCodecParameters()
{
    if (videoStreamIndex_ < 0 || !formatContext_)
        return nullptr;
    return formatContext_->streams[videoStreamIndex_]->codecpar;
}

AVCodecParameters *Demuxer::getAudioCodecParameters()
{
    if (audioStreamIndex_ < 0 || !formatContext_)
        return nullptr;
    return formatContext_->streams[audioStreamIndex_]->codecpar;
}

AVRational Demuxer::getVideoTimeBase()
{
    if (videoStreamIndex_ < 0 || !formatContext_)
        return {1, 1};
    return formatContext_->streams[videoStreamIndex_]->time_base;
}

AVRational Demuxer::getAudioTimeBase()
{
    if (audioStreamIndex_ < 0 || !formatContext_)
        return {1, 1};
    return formatContext_->streams[audioStreamIndex_]->time_base;
}
