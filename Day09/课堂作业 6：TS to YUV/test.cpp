extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
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
    const char* out_file = "output.yuv";

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
        cout << "未找到视频流" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecParameters* codec_par = fmt_ctx->streams[vid_idx]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
    if (!codec) {
        cout << "未找到解码器" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        cout << "无法分配解码器上下文" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    if (avcodec_parameters_to_context(codec_ctx, codec_par) < 0) {
        cout << "无法设置解码器参数" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cout << "无法打开解码器" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int w = codec_ctx->width;
    int h = codec_ctx->height;
    cout << "分辨率: " << w << "x" << h << endl;

    FILE* out_f = fopen(out_file, "wb");
    if (!out_f) {
        cout << "无法打开输出文件" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == vid_idx) {
            int ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret < 0) {
                cout << "发送包到解码器时出错" << endl;
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    cout << "从解码器接收帧时出错" << endl;
                    goto end;
                }

                for (int i = 0; i < frame->height; i++) {
                    fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, out_f);
                }

                for (int i = 0; i < frame->height / 2; i++) {
                    fwrite(frame->data[1] + i * frame->linesize[1], 1, frame->width / 2, out_f);
                }

                for (int i = 0; i < frame->height / 2; i++) {
                    fwrite(frame->data[2] + i * frame->linesize[2], 1, frame->width / 2, out_f);
                }
            }
        }
        av_packet_unref(pkt);
    }

end:
    fclose(out_f);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    cout << "解码完成，输出保存为: " << out_file << endl;
    cout << "播放命令: ffplay -f rawvideo -pixel_format yuv420p -video_size " 
         << w << "x" << h << " " << out_file << endl;
    return 0;
}
