extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}
#include <iostream>
using namespace std;

int main()
{
    const char *input = "/Users/leaderone/Desktop/study/MiCamp/Day09/课堂作业 5：/1.ts";

    AVFormatContext *fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, input, nullptr, nullptr) < 0)
    {
        cout << "无法打开文件: " << input << endl;
        perror("错误原因");
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
        cout << "无法获取流信息" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int v_idx = -1;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_idx = i;
            break;
        }
    }

    if (v_idx == -1)
    {
        cout << "找不到视频流" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecParameters *codecParams = fmt_ctx->streams[v_idx]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec)
    {
        cout << "找不到解码器" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        cout << "无法分配解码器上下文" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    if (avcodec_parameters_to_context(codec_ctx, codecParams) < 0)
    {
        cout << "无法复制解码器参数" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        cout << "无法打开解码器" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    int frame_count = 0;
    while (av_read_frame(fmt_ctx, pkt) >= 0)
    {
        if (pkt->stream_index == v_idx)
        {
            int ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret < 0)
            {
                cout << "发送数据包失败" << endl;
                break;
            }

            while (ret >= 0)
            {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    cout << "接收帧失败" << endl;
                    goto end;
                }

                frame_count++;
                cout << "解码第 " << frame_count << " 帧"
                     << " pts: " << frame->pts
                     << " 宽: " << frame->width
                     << " 高: " << frame->height << endl;
            }
        }
        av_packet_unref(pkt);
    }

end:
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    cout << "总共解码 " << frame_count << " 帧" << endl;
    return 0;
}
