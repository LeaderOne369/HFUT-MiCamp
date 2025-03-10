#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <functional>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "video_filter.h"
#include "video_decoder.h"
#include "videoencoder.h"
#include "muxer.h"

using namespace std;

void saveFrameToYUV(AVFrame *frame, FILE *file)
{
    if (!frame || !file)
        return;

    for (int i = 0; i < frame->height; i++)
    {
        fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, file);
    }

    for (int i = 0; i < frame->height / 2; i++)
    {
        fwrite(frame->data[1] + i * frame->linesize[1], 1, frame->width / 2, file);
    }

    for (int i = 0; i < frame->height / 2; i++)
    {
        fwrite(frame->data[2] + i * frame->linesize[2], 1, frame->width / 2, file);
    }
}

int main(int argc, char *argv[])
{
    int pipefd[2];
    if (pipe(pipefd) != -1)
    {
        int original_stderr = dup(STDERR_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        thread filterThread([pipefd, original_stderr]()
                            {
            FILE* pipe_read = fdopen(pipefd[0], "r");
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe_read) != NULL) {
                if (strstr(buffer, "too many B-frames in a row") == NULL) {
                    write(original_stderr, buffer, strlen(buffer));
                }
            }
            fclose(pipe_read);
            close(original_stderr); });
        filterThread.detach();
    }

    string inputFile = "resource/1.mp4";
    string outputYUV = "resource/output.yuv";
    string speedYUV = "resource/speed.yuv";
    string outputMP4 = "resource/speed.mp4";

    double speedFactor = 2.0;

    if (argc > 1)
    {
        inputFile = argv[1];
    }
    if (argc > 2)
    {
        outputMP4 = argv[2];
    }
    if (argc > 3)
    {
        int speedOption = atoi(argv[3]);
        switch (speedOption)
        {
        case 0:
            speedFactor = 0.5;
            break;
        case 1:
            speedFactor = 1.0;
            break;
        case 2:
            speedFactor = 2.0;
            break;
        case 3:
            speedFactor = 3.0;
            break;
        default:
            cerr << "无效的速度选项，使用默认值（2.0倍速）" << endl;
            speedFactor = 2.0;
            break;
        }
    }

    string outputDir;
    size_t lastSlash = outputMP4.find_last_of("/\\");
    if (lastSlash != string::npos)
    {
        outputDir = outputMP4.substr(0, lastSlash + 1);
        outputYUV = outputDir + "output.yuv";
        speedYUV = outputDir + "speed.yuv";
    }
    else
    {
        outputYUV = "output.yuv";
        speedYUV = "speed.yuv";
    }

    cout << "输入文件: " << inputFile << endl;
    cout << "输出YUV文件: " << outputYUV << endl;
    cout << "变速YUV文件: " << speedYUV << endl;
    cout << "输出MP4文件: " << outputMP4 << endl;
    cout << "视频速度: " << speedFactor << "倍速" << endl;

    FILE *outputYUVFile = fopen(outputYUV.c_str(), "wb");
    if (!outputYUVFile)
    {
        cerr << "无法创建输出YUV文件" << endl;
        return -1;
    }

    AVFormatContext *formatContext = nullptr;
    if (avformat_open_input(&formatContext, inputFile.c_str(), nullptr, nullptr) < 0)
    {
        cerr << "无法打开输入文件" << endl;
        fclose(outputYUVFile);
        return -1;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        cerr << "无法获取流信息" << endl;
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        cerr << "未找到视频流" << endl;
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    AVCodecParameters *codecParams = formatContext->streams[videoStreamIndex]->codecpar;

    const AVCodec *decoder = avcodec_find_decoder(codecParams->codec_id);
    if (!decoder)
    {
        cerr << "未找到解码器" << endl;
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    AVCodecContext *decoderContext = avcodec_alloc_context3(decoder);
    if (!decoderContext)
    {
        cerr << "无法分配解码器上下文" << endl;
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    if (avcodec_parameters_to_context(decoderContext, codecParams) < 0)
    {
        cerr << "无法复制编解码器参数" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    if (avcodec_open2(decoderContext, decoder, nullptr) < 0)
    {
        cerr << "无法打开解码器" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    VideoFilter videoFilter;

    int width = decoderContext->width;
    int height = decoderContext->height;
    AVPixelFormat pixFmt = decoderContext->pix_fmt;

    cout << "原始视频尺寸: " << width << "x" << height << endl;
    cout << "原始像素格式: " << av_get_pix_fmt_name(pixFmt) << endl;

    string filterDesc;
    if (speedFactor == 1.0)
    {
        filterDesc = "null";
    }
    else
    {
        filterDesc = "setpts=" + to_string(1.0 / speedFactor) + "*PTS";
    }

    if (!videoFilter.init(width, height, pixFmt, filterDesc))
    {
        cerr << "初始化视频过滤器失败" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    VideoEncoder videoEncoder;

    if (!videoEncoder.init(width, height, AV_PIX_FMT_YUV420P, "mpeg4", 1000000, 25))
    {
        cerr << "初始化视频编码器失败" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    Muxer muxer;

    if (!muxer.init(outputMP4))
    {
        cerr << "初始化封装器失败" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    if (!muxer.addVideoStream(videoEncoder.getCodecContext()))
    {
        cerr << "添加视频流失败" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    if (!muxer.writeHeader())
    {
        cerr << "写入文件头失败" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    int frameCount = 0;
    int filteredCount = 0;
    int encodedCount = 0;

    FILE *speedYUVFile = fopen(speedYUV.c_str(), "wb");
    if (!speedYUVFile)
    {
        cerr << "无法创建变速YUV文件" << endl;
        avcodec_free_context(&decoderContext);
        avformat_close_input(&formatContext);
        fclose(outputYUVFile);
        return -1;
    }

    videoFilter.setFrameCallback([&videoEncoder, &filteredCount, speedYUVFile](const VideoFrame *frame)
                                 {
        if (frame && frame->frame) {
            filteredCount++;
            
            saveFrameToYUV(frame->frame, speedYUVFile);
            videoEncoder.encodeFrame(frame);
        } });

    videoEncoder.setPacketCallback([&muxer, &encodedCount](VideoPacket *packet)
                                   {
        if (packet && packet->packet) {
            encodedCount++;
            
            muxer.writePacket(packet);
        } });

    cout << "开始解码、过滤和编码..." << endl;
    auto startTime = chrono::high_resolution_clock::now();

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    while (av_read_frame(formatContext, packet) >= 0)
    {
        if (packet->stream_index == videoStreamIndex)
        {
            int ret = avcodec_send_packet(decoderContext, packet);
            if (ret < 0)
            {
                cerr << "发送数据包到解码器失败" << endl;
                break;
            }

            while (ret >= 0)
            {
                ret = avcodec_receive_frame(decoderContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    cerr << "接收帧失败" << endl;
                    break;
                }

                frameCount++;

                saveFrameToYUV(frame, outputYUVFile);

                VideoFrame videoFrame;
                videoFrame.frame = av_frame_clone(frame);
                videoFrame.pts = frame->pts;

                videoFilter.processFrame(&videoFrame);

                if (videoFrame.frame)
                {
                    av_frame_free(&videoFrame.frame);
                }
            }
        }

        av_packet_unref(packet);
    }

    avcodec_send_packet(decoderContext, nullptr);
    while (true)
    {
        int ret = avcodec_receive_frame(decoderContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            cerr << "接收帧失败" << endl;
            break;
        }

        frameCount++;
        saveFrameToYUV(frame, outputYUVFile);

        VideoFrame videoFrame;
        videoFrame.frame = av_frame_clone(frame);
        videoFrame.pts = frame->pts;

        videoFilter.processFrame(&videoFrame);

        if (videoFrame.frame)
        {
            av_frame_free(&videoFrame.frame);
        }
    }

    videoEncoder.flush();
    muxer.writeTrailer();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    cout << "转码完成" << endl;
    cout << "处理时间: " << duration / 1000.0 << " 秒" << endl;
    cout << "总共解码帧数: " << frameCount << endl;
    cout << "总共过滤帧数: " << filteredCount << endl;
    cout << "总共编码帧数: " << encodedCount << endl;

    fclose(outputYUVFile);
    fclose(speedYUVFile);

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&decoderContext);
    avformat_close_input(&formatContext);
    muxer.close();

    cout << "\n使用ffplay查看YUV文件:" << endl;
    cout << "ffplay -f rawvideo -pixel_format yuv420p -video_size " << width << "x" << height
         << " " << outputYUV << endl;
    cout << "ffplay -f rawvideo -pixel_format yuv420p -video_size " << width << "x" << height
         << " " << speedYUV << endl;

    return 0;
}
