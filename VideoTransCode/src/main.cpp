#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "../include/transcode.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "用法: " << argv[0] << " <输入文件> [<输出视频文件> <输出音频文件>]" << endl;
        return -1;
    }

    string inputFile = argv[1];
    string outputVideoFile = (argc > 2) ? argv[2] : "resource/output.yuv";
    string outputAudioFile = (argc > 3) ? argv[3] : "resource/output.pcm";

    cout << "输入文件: " << inputFile << endl;
    cout << "输出视频文件: " << outputVideoFile << endl;
    cout << "输出音频文件: " << outputAudioFile << endl;

    Transcode transcode(inputFile, outputVideoFile, outputAudioFile);

    if (!transcode.init())
    {
        cerr << "初始化转码器失败" << endl;
        return -1;
    }

    cout << "开始转码..." << endl;

    auto startTime = chrono::high_resolution_clock::now();

    if (!transcode.start())
    {
        cerr << "启动转码失败" << endl;
        return -1;
    }

    cout << "正在处理，请稍候..." << endl;
#ifdef __linux__
    cout << "在 Ubuntu 系统上运行，增加等待时间..." << endl;
    this_thread::sleep_for(chrono::seconds(10));
#else
    this_thread::sleep_for(chrono::seconds(5));
#endif

    transcode.waitForCompletion();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    cout << "转码完成" << endl;
    cout << "处理时间: " << duration / 1000.0 << " 秒" << endl;
    cout << "视频帧数: " << transcode.getFrameCount() << endl;
    cout << "音频样本数: " << transcode.getSampleCount() << endl;

    cout << "\n使用ffplay播放视频:" << endl;
    cout << "ffplay -f rawvideo -pixel_format yuv420p -video_size 1024x436 " << outputVideoFile << endl;

    cout << "\n使用ffplay播放音频:" << endl;
    cout << "ffplay -f s16le -ar 48000 -ch_layout stereo " << outputAudioFile << endl;

    return 0;
}