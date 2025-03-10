#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <algorithm>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
#include <libavutil/version.h>
#include <libavutil/mathematics.h>
}

#include "muxer.h"
#include "videoencoder.h"

using namespace std;

static inline int get_channels(const AVCodecContext *ctx)
{
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100)
    return ctx->ch_layout.nb_channels;
#else
#ifdef __APPLE__
    if (ctx->channel_layout)
    {
        int nb_channels = 0;
        for (uint64_t i = 0; i < 64; i++)
        {
            if ((ctx->channel_layout & (1ULL << i)) != 0)
            {
                nb_channels++;
            }
        }
        return nb_channels > 0 ? nb_channels : ctx->channels;
    }
#endif
    return ctx->channels;
#endif
}

static char *ts2str(int64_t ts, AVRational *tb)
{
    static char str[AV_TS_MAX_STRING_SIZE];
    if (ts == AV_NOPTS_VALUE)
    {
        snprintf(str, AV_TS_MAX_STRING_SIZE, "NOPTS");
    }
    else
    {
        av_ts_make_time_string(str, ts, tb);
    }
    return str;
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           ts2str(pkt->pts, time_base), ts2str(pkt->pts, time_base),
           ts2str(pkt->dts, time_base), ts2str(pkt->dts, time_base),
           ts2str(pkt->duration, time_base), ts2str(pkt->duration, time_base),
           pkt->stream_index);
}

static int64_t rescale_ts(int64_t ts, AVRational src_tb, AVRational dst_tb)
{
    return av_rescale_q(ts, src_tb, dst_tb);
}

int main(int argc, char *argv[])
{
    string videoFile = "resource/speed.mp4";
    string audioFile = "resource/speed.ac3";
    string outputFile = "resource/mi.mp4";

    if (argc > 1)
    {
        videoFile = argv[1];
    }
    if (argc > 2)
    {
        audioFile = argv[2];
    }
    if (argc > 3)
    {
        outputFile = argv[3];
    }

    cout << "===== 音视频封装 =====" << endl;
    cout << "视频文件: " << videoFile << endl;
    cout << "音频文件: " << audioFile << endl;
    cout << "输出文件: " << outputFile << endl;

    AVFormatContext *videoFormatCtx = nullptr;
    int ret = avformat_open_input(&videoFormatCtx, videoFile.c_str(), nullptr, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法打开视频文件: " << errBuf << endl;
        return 1;
    }

    ret = avformat_find_stream_info(videoFormatCtx, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法获取视频流信息: " << errBuf << endl;
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < videoFormatCtx->nb_streams; i++)
    {
        if (videoFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        cerr << "未找到视频流" << endl;
        cerr << "文件中包含的流数量: " << videoFormatCtx->nb_streams << endl;
        for (unsigned int i = 0; i < videoFormatCtx->nb_streams; i++)
        {
            cerr << "流 " << i << " 类型: " << videoFormatCtx->streams[i]->codecpar->codec_type << endl;
        }
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVCodecParameters *videoCodecParams = videoFormatCtx->streams[videoStreamIndex]->codecpar;

    const AVCodec *videoCodec = avcodec_find_decoder(videoCodecParams->codec_id);
    if (!videoCodec)
    {
        cerr << "未找到视频解码器" << endl;
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVCodecContext *videoCodecCtx = avcodec_alloc_context3(videoCodec);
    if (!videoCodecCtx)
    {
        cerr << "无法分配视频解码器上下文" << endl;
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    ret = avcodec_parameters_to_context(videoCodecCtx, videoCodecParams);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法复制视频编解码器参数: " << errBuf << endl;
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    ret = avcodec_open2(videoCodecCtx, videoCodec, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法打开视频解码器: " << errBuf << endl;
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVFormatContext *audioFormatCtx = nullptr;
    ret = avformat_open_input(&audioFormatCtx, audioFile.c_str(), nullptr, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法打开音频文件: " << errBuf << endl;
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    ret = avformat_find_stream_info(audioFormatCtx, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法获取音频流信息: " << errBuf << endl;
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < audioFormatCtx->nb_streams; i++)
    {
        if (audioFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1)
    {
        cerr << "未找到音频流" << endl;
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVCodecParameters *audioCodecParams = audioFormatCtx->streams[audioStreamIndex]->codecpar;

    const AVCodec *audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    if (!audioCodec)
    {
        cerr << "未找到音频解码器" << endl;
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVCodecContext *audioCodecCtx = avcodec_alloc_context3(audioCodec);
    if (!audioCodecCtx)
    {
        cerr << "无法分配音频解码器上下文" << endl;
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    ret = avcodec_parameters_to_context(audioCodecCtx, audioCodecParams);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法复制音频编解码器参数: " << errBuf << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    ret = avcodec_open2(audioCodecCtx, audioCodec, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cerr << "无法打开音频解码器: " << errBuf << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVRational videoFrameRate = videoFormatCtx->streams[videoStreamIndex]->avg_frame_rate;
    if (videoFrameRate.num == 0 || videoFrameRate.den == 0)
    {
        videoFrameRate = videoFormatCtx->streams[videoStreamIndex]->r_frame_rate;
    }

    cout << "\n视频信息:" << endl;
    cout << "编解码器: " << videoCodec->name << endl;
    cout << "分辨率: " << videoCodecCtx->width << "x" << videoCodecCtx->height << endl;
    cout << "帧率: " << videoFrameRate.num << "/" << videoFrameRate.den << " ("
         << av_q2d(videoFrameRate) << " fps)" << endl;
    cout << "时基: " << videoFormatCtx->streams[videoStreamIndex]->time_base.num << "/"
         << videoFormatCtx->streams[videoStreamIndex]->time_base.den << endl;
    cout << "总时长: " << videoFormatCtx->duration / AV_TIME_BASE << " 秒" << endl;

    cout << "\n音频信息:" << endl;
    cout << "编解码器: " << audioCodec->name << endl;
    cout << "采样率: " << audioCodecCtx->sample_rate << " Hz" << endl;
    cout << "通道数: " << get_channels(audioCodecCtx) << endl;
    cout << "时基: " << audioFormatCtx->streams[audioStreamIndex]->time_base.num << "/"
         << audioFormatCtx->streams[audioStreamIndex]->time_base.den << endl;
    cout << "总时长: " << audioFormatCtx->duration / AV_TIME_BASE << " 秒" << endl;

    Muxer muxer;
    if (!muxer.init(outputFile))
    {
        cerr << "初始化封装器失败" << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    if (!muxer.addVideoStream(videoCodecCtx))
    {
        cerr << "添加视频流失败" << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    if (!muxer.addAudioStream(audioCodecCtx))
    {
        cerr << "添加音频流失败" << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }
    if (!muxer.writeHeader())
    {
        cerr << "写入文件头失败" << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        cerr << "无法分配数据包" << endl;
        avcodec_free_context(&audioCodecCtx);
        avformat_close_input(&audioFormatCtx);
        avcodec_free_context(&videoCodecCtx);
        avformat_close_input(&videoFormatCtx);
        return 1;
    }

    auto startTime = chrono::high_resolution_clock::now();

    AVRational videoOutTimeBase = muxer.getFormatContext()->streams[0]->time_base;
    AVRational audioOutTimeBase = muxer.getFormatContext()->streams[1]->time_base;

    AVRational videoInTimeBase = videoFormatCtx->streams[videoStreamIndex]->time_base;
    AVRational audioInTimeBase = audioFormatCtx->streams[audioStreamIndex]->time_base;

    cout << "\n输出视频时基: " << videoOutTimeBase.num << "/" << videoOutTimeBase.den << endl;
    cout << "输出音频时基: " << audioOutTimeBase.num << "/" << audioOutTimeBase.den << endl;

    vector<AVPacket *> videoPackets;
    vector<AVPacket *> audioPackets;

    cout << "\n读取视频数据包..." << endl;
    av_seek_frame(videoFormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
    while (av_read_frame(videoFormatCtx, packet) >= 0)
    {
        if (packet->stream_index == videoStreamIndex)
        {
            AVPacket *newPacket = av_packet_alloc();
            av_packet_ref(newPacket, packet);
            videoPackets.push_back(newPacket);
        }
        av_packet_unref(packet);
    }
    cout << "读取了 " << videoPackets.size() << " 个视频数据包" << endl;

    cout << "\n读取音频数据包..." << endl;
    av_seek_frame(audioFormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
    while (av_read_frame(audioFormatCtx, packet) >= 0)
    {
        if (packet->stream_index == audioStreamIndex)
        {
            AVPacket *newPacket = av_packet_alloc();
            av_packet_ref(newPacket, packet);
            audioPackets.push_back(newPacket);
        }
        av_packet_unref(packet);
    }
    cout << "读取了 " << audioPackets.size() << " 个音频数据包" << endl;

    double videoDuration = 0;
    if (!videoPackets.empty())
    {
        AVPacket *lastPacket = videoPackets.back();
        int64_t lastPts = lastPacket->pts + lastPacket->duration;
        videoDuration = lastPts * av_q2d(videoInTimeBase);
    }

    double audioDuration = 0;
    if (!audioPackets.empty())
    {
        AVPacket *lastPacket = audioPackets.back();
        int64_t lastPts = lastPacket->pts + lastPacket->duration;
        audioDuration = lastPts * av_q2d(audioInTimeBase);
    }

    cout << "\n视频时长: " << videoDuration << " 秒" << endl;
    cout << "音频时长: " << audioDuration << " 秒" << endl;

    double targetDuration = min(videoDuration, audioDuration);
    cout << "目标时长: " << targetDuration << " 秒" << endl;

    cout << "\n开始交错写入数据包..." << endl;

    AVRational outputFrameRate = {videoFrameRate.num, videoFrameRate.den};
    AVRational outputTimeBase = {1, 90000};

    cout << "输出帧率: " << outputFrameRate.num << "/" << outputFrameRate.den << " ("
         << av_q2d(outputFrameRate) << " fps)" << endl;
    cout << "输出时基: " << outputTimeBase.num << "/" << outputTimeBase.den << endl;

    int videoIndex = 0;
    int audioIndex = 0;
    int totalWritten = 0;

    double videoTime = 0;
    double audioTime = 0;

    int64_t nextVideoPts = 0;
    int64_t nextAudioPts = 0;

    int64_t videoFrameDuration = av_rescale_q(1, av_inv_q(outputFrameRate), outputTimeBase);

    cout << "视频帧持续时间: " << videoFrameDuration << " (输出时基单位)" << endl;
    while (videoIndex < videoPackets.size() || audioIndex < audioPackets.size())
    {
        bool writeVideo = false;

        if (videoIndex >= videoPackets.size())
        {
            writeVideo = false;
        }
        else if (audioIndex >= audioPackets.size())
        {
            writeVideo = true;
        }
        else
        {
            writeVideo = (videoTime <= audioTime);
        }

        if (writeVideo && videoIndex < videoPackets.size())
        {
            AVPacket *videoPacket = videoPackets[videoIndex];

            int64_t pts = av_rescale_q(nextVideoPts, outputTimeBase, videoOutTimeBase);
            int64_t dts = pts;

            VideoPacket *outPacket = new VideoPacket();
            outPacket->packet = av_packet_clone(videoPacket);
            outPacket->packet->pts = pts;
            outPacket->packet->dts = dts;
            outPacket->packet->duration = av_rescale_q(videoFrameDuration, outputTimeBase, videoOutTimeBase);
            outPacket->streamIndex = 0;

            if (!muxer.writePacket(outPacket))
            {
                cerr << "写入视频数据包失败" << endl;
                delete outPacket;
                break;
            }

            delete outPacket;
            videoIndex++;
            totalWritten++;

            nextVideoPts += videoFrameDuration;
            videoTime = nextVideoPts * av_q2d(outputTimeBase);

            if (totalWritten % 100 == 0)
            {
                cout << "已写入 " << totalWritten << " 个数据包\r" << flush;
            }
        }
        else if (!writeVideo && audioIndex < audioPackets.size())
        {
            AVPacket *audioPacket = audioPackets[audioIndex];

            int64_t pts = av_rescale_q(nextAudioPts, outputTimeBase, audioOutTimeBase);
            int64_t dts = pts;
            int64_t duration = av_rescale_q(audioPacket->duration, audioInTimeBase, audioOutTimeBase);

            VideoPacket *outPacket = new VideoPacket();
            outPacket->packet = av_packet_clone(audioPacket);
            outPacket->packet->pts = pts;
            outPacket->packet->dts = dts;
            outPacket->packet->duration = duration;
            outPacket->streamIndex = 1;
            if (!muxer.writePacket(outPacket))
            {
                cerr << "写入音频数据包失败" << endl;
                delete outPacket;
                break;
            }

            delete outPacket;
            audioIndex++;
            totalWritten++;

            nextAudioPts += av_rescale_q(audioPacket->duration, audioInTimeBase, outputTimeBase);
            audioTime = nextAudioPts * av_q2d(outputTimeBase);

            if (totalWritten % 100 == 0)
            {
                cout << "已写入 " << totalWritten << " 个数据包\r" << flush;
            }
        }
    }

    cout << "\n数据包写入完成，共写入 " << totalWritten << " 个数据包" << endl;
    cout << "视频包: " << videoIndex << endl;
    cout << "音频包: " << audioIndex << endl;

    if (!muxer.writeTrailer())
    {
        cerr << "写入文件尾失败" << endl;
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() / 1000.0;

    cout << "\n封装完成!" << endl;
    cout << "输出文件: " << outputFile << endl;
    cout << "处理时间: " << duration << " 秒" << endl;

    for (auto p : videoPackets)
    {
        av_packet_free(&p);
    }
    for (auto p : audioPackets)
    {
        av_packet_free(&p);
    }

    av_packet_free(&packet);
    avcodec_free_context(&audioCodecCtx);
    avformat_close_input(&audioFormatCtx);
    avcodec_free_context(&videoCodecCtx);
    avformat_close_input(&videoFormatCtx);

    cout << "\n您可以使用以下命令播放生成的文件:" << endl;
    cout << "ffplay " << outputFile << endl;

    return 0;
}