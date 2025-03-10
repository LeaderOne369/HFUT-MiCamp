#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include "audio_filter.h"
#include "audioencoder.h"
#include "queue.h"
#include "demuxer.h"
#include "audio_decoder.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

using namespace std;

class AC3Writer
{
private:
    ofstream outputFile;

public:
    AC3Writer(const string &filename)
    {
        outputFile.open(filename, ios::binary);
        if (!outputFile.is_open())
        {
            cerr << "无法打开输出文件: " << filename << endl;
        }
    }

    ~AC3Writer()
    {
        if (outputFile.is_open())
        {
            outputFile.close();
        }
    }

    bool isOpen() const
    {
        return outputFile.is_open();
    }

    void writePacket(AVPacket *packet)
    {
        if (!outputFile.is_open() || !packet)
        {
            return;
        }

        outputFile.write(reinterpret_cast<const char *>(packet->data), packet->size);
    }
};

class MP4Writer
{
private:
    AVFormatContext *formatContext_ = nullptr;
    int streamIndex_ = -1;
    int64_t nextPts_ = 0;
    AVRational timeBase_;

public:
    MP4Writer(const string &filename, AVCodecContext *codecContext)
    {
        avformat_alloc_output_context2(&formatContext_, nullptr, nullptr, filename.c_str());
        if (!formatContext_)
        {
            cerr << "无法创建输出格式上下文" << endl;
            return;
        }

        AVStream *stream = avformat_new_stream(formatContext_, nullptr);
        if (!stream)
        {
            cerr << "无法创建音频流" << endl;
            return;
        }

        streamIndex_ = stream->index;
        avcodec_parameters_from_context(stream->codecpar, codecContext);
        stream->time_base = codecContext->time_base;
        timeBase_ = stream->time_base;

        if (!(formatContext_->oformat->flags & AVFMT_NOFILE))
        {
            int ret = avio_open(&formatContext_->pb, filename.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0)
            {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                cerr << "无法打开输出文件: " << errbuf << endl;
                return;
            }
        }

        int ret = avformat_write_header(formatContext_, nullptr);
        if (ret < 0)
        {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            cerr << "无法写入文件头: " << errbuf << endl;
            return;
        }

        cout << "MP4文件初始化成功" << endl;
    }

    ~MP4Writer()
    {
        if (formatContext_)
        {
            av_write_trailer(formatContext_);

            if (!(formatContext_->oformat->flags & AVFMT_NOFILE))
            {
                avio_closep(&formatContext_->pb);
            }

            avformat_free_context(formatContext_);
        }
    }

    bool isOpen() const
    {
        return formatContext_ != nullptr && streamIndex_ >= 0;
    }

    void writePacket(AVPacket *packet)
    {
        if (!isOpen() || !packet)
        {
            return;
        }

        packet->stream_index = streamIndex_;
        packet->pts = nextPts_;
        packet->dts = nextPts_;
        nextPts_ += packet->duration;

        int ret = av_interleaved_write_frame(formatContext_, packet);
        if (ret < 0)
        {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            cerr << "无法写入数据包: " << errbuf << endl;
        }
    }
};

class PCMProcessor
{
private:
    ofstream outputFile;
    int framesProcessed = 0;

public:
    PCMProcessor(const string &outputPath)
    {
        outputFile.open(outputPath, ios::binary);
        if (!outputFile.is_open())
        {
            cerr << "无法打开输出文件: " << outputPath << endl;
        }
    }

    ~PCMProcessor()
    {
        if (outputFile.is_open())
        {
            outputFile.close();
        }
        cout << "总共处理了 " << framesProcessed << " 帧音频" << endl;
    }

    bool isOpen() const
    {
        return outputFile.is_open();
    }

    void processFrame(const AudioFrame *frame)
    {
        if (!frame || !frame->data || frame->size <= 0)
        {
            return;
        }

        if (outputFile.is_open())
        {
            outputFile.write(reinterpret_cast<const char *>(frame->data), frame->size);
            framesProcessed++;
        }

        delete frame;
    }
};

enum class AudioSpeed
{
    HALF_SPEED = 0,
    NORMAL_SPEED = 1,
    DOUBLE_SPEED = 2,
    TRIPLE_SPEED = 3
};

string getSpeedFilterDesc(AudioSpeed speed)
{
    switch (speed)
    {
    case AudioSpeed::HALF_SPEED:
        return "atempo=0.5";
    case AudioSpeed::NORMAL_SPEED:
        return "anull";
    case AudioSpeed::DOUBLE_SPEED:
        return "atempo=2.0";
    case AudioSpeed::TRIPLE_SPEED:
        return "atempo=2.0,atempo=1.5";
    default:
        return "anull";
    }
}

string getSpeedDescription(AudioSpeed speed)
{
    switch (speed)
    {
    case AudioSpeed::HALF_SPEED:
        return "0.5倍速";
    case AudioSpeed::NORMAL_SPEED:
        return "1.0倍速";
    case AudioSpeed::DOUBLE_SPEED:
        return "2.0倍速";
    case AudioSpeed::TRIPLE_SPEED:
        return "3.0倍速";
    default:
        return "未知速度";
    }
}

string getSpeedFileSuffix(AudioSpeed speed)
{
    switch (speed)
    {
    case AudioSpeed::HALF_SPEED:
        return "_0.5x";
    case AudioSpeed::NORMAL_SPEED:
        return "_1.0x";
    case AudioSpeed::DOUBLE_SPEED:
        return "_2.0x";
    case AudioSpeed::TRIPLE_SPEED:
        return "_3.0x";
    default:
        return "";
    }
}

class PCMBuffer
{
private:
    vector<uint8_t> buffer;
    int bytesPerSample;
    int channels;
    int frameSize;
    int bytesPerFrame;

public:
    PCMBuffer(int bytesPerSample, int channels, int frameSize)
        : bytesPerSample(bytesPerSample), channels(channels), frameSize(frameSize)
    {
        bytesPerFrame = bytesPerSample * channels * frameSize;
        buffer.reserve(bytesPerFrame * 2);
    }

    void addData(const uint8_t *data, int size)
    {
        buffer.insert(buffer.end(), data, data + size);
    }

    bool getFrame(vector<uint8_t> &frame)
    {
        if (buffer.size() < bytesPerFrame)
        {
            return false;
        }

        frame.assign(buffer.begin(), buffer.begin() + bytesPerFrame);
        buffer.erase(buffer.begin(), buffer.begin() + bytesPerFrame);
        return true;
    }

    void getRemainingData(vector<uint8_t> &frame)
    {
        if (buffer.empty())
        {
            return;
        }

        frame = buffer;
        int paddingSize = bytesPerFrame - frame.size();
        if (paddingSize > 0)
        {
            frame.resize(bytesPerFrame, 0);
        }
        buffer.clear();
    }

    size_t size() const
    {
        return buffer.size();
    }

    void clear()
    {
        buffer.clear();
    }
};

int main(int argc, char *argv[])
{
    string inputMP4 = "resource/1.mp4";
    string outputDir = "resource";
    int sampleRate = 48000;
    int channels = 2;
    AVSampleFormat format = AV_SAMPLE_FMT_S16;
    string codecName = "ac3";
    int bitrate = 192000;

    AudioSpeed selectedSpeed = AudioSpeed::DOUBLE_SPEED;

    if (argc > 1)
    {
        inputMP4 = argv[1];
    }
    if (argc > 2)
    {
        outputDir = argv[2];
    }
    if (argc > 3)
    {
        int speedOption = atoi(argv[3]);
        if (speedOption >= 0 && speedOption <= 3)
        {
            selectedSpeed = static_cast<AudioSpeed>(speedOption);
        }
        else
        {
            cerr << "无效的速度选项，使用默认值（2.0倍速）" << endl;
        }
    }

    string dirPath = outputDir;
    string suffix = "";

    size_t underscorePos = outputDir.find_last_of("_");
    if (underscorePos != string::npos)
    {
        dirPath = outputDir.substr(0, underscorePos);
        suffix = outputDir.substr(underscorePos);
    }

    if (!dirPath.empty() && dirPath.back() != '/' && dirPath.back() != '\\')
    {
        dirPath += '/';
    }

    string outputPCM = dirPath + "output" + suffix + ".pcm";
    string speedPCM = dirPath + "speed" + suffix + ".pcm";
    string outputAC3 = dirPath + "speed" + suffix + ".ac3";

    string filterDesc = getSpeedFilterDesc(selectedSpeed);
    string speedDesc = getSpeedDescription(selectedSpeed);

    cout << "===== 第一阶段：从MP4解码音频 =====" << endl;
    cout << "输入MP4文件: " << inputMP4 << endl;
    cout << "输出PCM文件: " << outputPCM << endl;
    cout << "音频速度: " << speedDesc << endl;

    auto audioQueue = make_shared<AudioPacketQueue>();

    Demuxer demuxer(inputMP4, nullptr, audioQueue);
    if (!demuxer.init())
    {
        cerr << "初始化解复用器失败" << endl;
        return 1;
    }

    AVCodecParameters *audioCodecParams = demuxer.getAudioCodecParameters();
    if (!audioCodecParams)
    {
        cerr << "无法获取音频编解码器参数" << endl;
        return 1;
    }

    AVRational audioTimeBase = demuxer.getAudioTimeBase();

    PCMProcessor pcmExtractor(outputPCM);
    if (!pcmExtractor.isOpen())
    {
        cerr << "无法创建PCM提取器" << endl;
        return 1;
    }

    AudioDecoder audioDecoder(audioQueue, audioCodecParams, audioTimeBase);

    audioDecoder.setOutputFormat(sampleRate, channels, format);

    audioDecoder.setFrameCallback([&pcmExtractor](AudioFramePtr frame)
                                  {
        if (frame) {
            AudioFrame* newFrame = new AudioFrame();
            newFrame->data = new uint8_t[frame->size];
            memcpy(newFrame->data, frame->data, frame->size);
            newFrame->size = frame->size;
            newFrame->sampleRate = frame->sampleRate;
            newFrame->channels = frame->channels;
            newFrame->pts = frame->pts;
            
            pcmExtractor.processFrame(newFrame);
        } });

    if (!audioDecoder.init())
    {
        cerr << "初始化音频解码器失败" << endl;
        return 1;
    }

    demuxer.start();
    audioDecoder.start();

    cout << "正在解码音频，请稍候..." << endl;

    this_thread::sleep_for(chrono::seconds(5));

    audioDecoder.stop();
    demuxer.stop();

    cout << "音频解码完成" << endl;

    cout << "\n===== 第二阶段：音频滤镜处理 =====" << endl;
    cout << "输入PCM文件: " << outputPCM << endl;
    cout << "输出处理后PCM文件: " << speedPCM << endl;
    cout << "采样率: " << sampleRate << " Hz" << endl;
    cout << "通道数: " << channels << endl;
    cout << "采样格式: " << av_get_sample_fmt_name(format) << endl;
    cout << "音频速度: " << speedDesc << endl;
    cout << "滤镜描述: " << filterDesc << endl;

    ifstream inFile(outputPCM, ios::binary);
    if (!inFile.is_open())
    {
        cerr << "无法打开输入文件: " << outputPCM << endl;
        return 1;
    }

    PCMProcessor pcmProcessor(speedPCM);
    if (!pcmProcessor.isOpen())
    {
        inFile.close();
        return 1;
    }

    AudioFilter filter;
    if (!filter.init(sampleRate, channels, format, filterDesc))
    {
        cerr << "初始化音频滤镜失败" << endl;
        inFile.close();
        return 1;
    }

    filter.setFrameCallback([&pcmProcessor](const AudioFrame *frame)
                            { pcmProcessor.processFrame(frame); });

    int bytesPerSample = av_get_bytes_per_sample(format);
    int bytesPerFrame = bytesPerSample * channels;

    const int samplesPerChunk = 1024;
    const int chunkSize = samplesPerChunk * bytesPerFrame;

    vector<uint8_t> buffer(chunkSize);

    int64_t pts = 0;
    int framesProcessed = 0;

    while (inFile)
    {
        inFile.read(reinterpret_cast<char *>(buffer.data()), chunkSize);
        int bytesRead = inFile.gcount();

        if (bytesRead <= 0)
        {
            break;
        }

        AudioFrame *frame = new AudioFrame();
        frame->data = new uint8_t[bytesRead];
        memcpy(frame->data, buffer.data(), bytesRead);
        frame->size = bytesRead;
        frame->sampleRate = sampleRate;
        frame->channels = channels;
        frame->pts = pts;

        if (!filter.processFrame(frame))
        {
            cerr << "处理音频帧失败" << endl;
            delete frame;
            break;
        }

        int samplesRead = bytesRead / bytesPerFrame;
        pts += samplesRead;
        framesProcessed++;

        delete frame;
    }

    inFile.close();

    cout << "音频滤镜处理完成，处理了 " << framesProcessed << " 个音频块" << endl;
    cout << "输出文件: " << speedPCM << endl;

    cout << "\n===== 第三阶段：音频编码 =====" << endl;
    cout << "输入PCM文件: " << speedPCM << endl;
    cout << "输出AC3文件: " << outputAC3 << endl;
    cout << "编码器: " << codecName << endl;
    cout << "比特率: " << bitrate << " bps" << endl;

    ifstream filteredFile(speedPCM, ios::binary);
    if (!filteredFile.is_open())
    {
        cerr << "无法打开处理后的PCM文件: " << speedPCM << endl;
        return 1;
    }

    AC3Writer ac3Writer(outputAC3);
    if (!ac3Writer.isOpen())
    {
        filteredFile.close();
        return 1;
    }

    AudioEncoder encoder;
    if (!encoder.init(sampleRate, channels, format, codecName, bitrate))
    {
        cerr << "初始化音频编码器失败" << endl;
        filteredFile.close();
        return 1;
    }

    int encoderFrameSize = encoder.getCodecContext()->frame_size;
    cout << "编码器帧大小: " << encoderFrameSize << " 样本" << endl;

    encoder.setPacketCallback([&ac3Writer](AVPacket *packet)
                              {
        ac3Writer.writePacket(packet);
        
        av_packet_free(&packet); });

    PCMBuffer pcmBuffer(bytesPerSample, channels, encoderFrameSize);

    buffer.resize(chunkSize);
    pts = 0;
    framesProcessed = 0;

    while (filteredFile)
    {
        filteredFile.read(reinterpret_cast<char *>(buffer.data()), chunkSize);
        int bytesRead = filteredFile.gcount();

        if (bytesRead <= 0)
        {
            break;
        }

        pcmBuffer.addData(buffer.data(), bytesRead);

        vector<uint8_t> frameData;
        while (pcmBuffer.getFrame(frameData))
        {
            if (!encoder.encodeFrame(frameData.data(), frameData.size(), pts))
            {
                cerr << "编码帧失败" << endl;
                break;
            }

            pts += encoderFrameSize;
            framesProcessed++;
        }
    }

    vector<uint8_t> lastFrame;
    pcmBuffer.getRemainingData(lastFrame);
    if (!lastFrame.empty())
    {
        if (!encoder.encodeFrame(lastFrame.data(), lastFrame.size(), pts))
        {
            cerr << "编码最后一帧失败" << endl;
        }
        else
        {
            framesProcessed++;
        }
    }

    encoder.flush();

    filteredFile.close();

    cout << "音频编码完成，处理了 " << framesProcessed << " 个音频块" << endl;
    cout << "输出AC3文件: " << outputAC3 << endl;

    cout << "\n您可以使用以下命令播放生成的文件:" << endl;
    cout << "ffplay -f s16le -ar " << sampleRate << " -ac " << channels << " " << outputPCM << endl;
    cout << "ffplay -f s16le -ar " << sampleRate << " -ac " << channels << " " << speedPCM << endl;
    cout << "ffplay " << outputAC3 << endl;

    return 0;
}
