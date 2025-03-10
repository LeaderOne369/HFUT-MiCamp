extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "用法: " << argv[0] << " <输入文件>" << endl;
        return -1;
    }

    const char* in_file = argv[1];
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, in_file, nullptr, nullptr) < 0) {
        cout << "无法打开输入文件" << endl;
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        cout << "无法找到流信息" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int vid_idx = -1;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vid_idx = i;
            break;
        }
    }

    if (vid_idx == -1) {
        cout << "没有找到视频流" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        cout << "无法分配AVPacket" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int pkt_cnt = 0;
    while (av_read_frame(fmt_ctx, pkt) >= 0 && pkt_cnt < 100) {
        if (pkt->stream_index == vid_idx) {
            pkt_cnt++;
            cout << "\n包 #" << pkt_cnt << " 信息:" << endl;
            cout << "DTS: " << pkt->dts 
                 << ", PTS: " << pkt->pts 
                 << ", 持续时间: " << pkt->duration << endl;

            AVRational time_base = fmt_ctx->streams[vid_idx]->time_base;
            double dts_sec = (pkt->dts != AV_NOPTS_VALUE) ? 
                             pkt->dts * av_q2d(time_base) : -1;
            double pts_sec = (pkt->pts != AV_NOPTS_VALUE) ? 
                             pkt->pts * av_q2d(time_base) : -1;

            cout << "DTS(秒): " << dts_sec 
                 << ", PTS(秒): " << pts_sec << endl;
            cout << "------------------------" << endl;
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    avformat_close_input(&fmt_ctx);

    cout << "\n分析完成，读取了 " << pkt_cnt << " 个视频包" << endl;
    return 0;
}
