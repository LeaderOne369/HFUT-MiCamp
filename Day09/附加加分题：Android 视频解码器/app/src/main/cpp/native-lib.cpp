#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "FFmpegDecoder", "keyword: " __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "FFmpegDecoder", "keyword: " __VA_ARGS__)

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_extra_MainActivity_decodeToYUV(JNIEnv *env, jobject thiz, jstring input_path, jstring output_path) {
    const char *input_file = env->GetStringUTFChars(input_path, nullptr);
    const char *output_file = env->GetStringUTFChars(output_path, nullptr);
    
    LOGI("开始解码文件: %s", input_file);
    
    // 尝试直接读取文件
    FILE* test_file = fopen(input_file, "rb");
    if (!test_file) {
        LOGE("fopen失败，errno: %d (%s)", errno, strerror(errno));
    } else {
        fclose(test_file);
    }
    
    // 打开输入文件
    AVFormatContext* format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, input_file, nullptr, nullptr) < 0) {
        LOGE("无法打开输入文件: %s", input_file);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("成功打开输入文件");

    // 获取流信息
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        LOGE("无法获取流信息");
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("成功获取流信息");

    // 找到视频流
    int video_stream_index = -1;
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        LOGE("未找到视频流");
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("找到视频流，索引: %d", video_stream_index);

    // 获取解码器
    AVCodecParameters* codec_params = format_ctx->streams[video_stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        LOGE("未找到解码器");
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("找到解码器: %s", codec->name);

    // 创建解码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        LOGE("无法创建解码器上下文");
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }

    // 设置解码器参数
    if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
        LOGE("无法设置解码器参数");
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }

    // 打开解码器
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        LOGE("无法打开解码器");
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("成功打开解码器");

    // 打开输出文件
    FILE* outfile = fopen(output_file, "wb");
    if (!outfile) {
        LOGE("无法创建输出文件: %s", output_file);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("成功创建输出文件");

    // 分配帧缓冲
    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    if (!frame || !packet) {
        LOGE("无法分配帧缓冲");
        if (frame) av_frame_free(&frame);
        if (packet) av_packet_free(&packet);
        fclose(outfile);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        env->ReleaseStringUTFChars(input_path, input_file);
        env->ReleaseStringUTFChars(output_path, output_file);
        return -1;
    }
    LOGI("成功分配帧缓冲");

    // 开始解码
    int frame_count = 0;
    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            int ret = avcodec_send_packet(codec_ctx, packet);
            if (ret < 0) {
                LOGE("发送数据包到解码器失败");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    LOGE("从解码器接收帧失败");
                    goto end;
                }

                // 写入YUV数据
                for (int i = 0; i < frame->height; i++) {
                    fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, outfile);
                }
                for (int i = 0; i < frame->height/2; i++) {
                    fwrite(frame->data[1] + i * frame->linesize[1], 1, frame->width/2, outfile);
                }
                for (int i = 0; i < frame->height/2; i++) {
                    fwrite(frame->data[2] + i * frame->linesize[2], 1, frame->width/2, outfile);
                }
                
                frame_count++;
                LOGI("成功解码第 %d 帧", frame_count);
            }
        }
        av_packet_unref(packet);
    }

end:
    // 清理资源
    LOGI("解码完成，共解码 %d 帧", frame_count);
    fclose(outfile);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    env->ReleaseStringUTFChars(input_path, input_file);
    env->ReleaseStringUTFChars(output_path, output_file);

    return frame_count > 0 ? 0 : -1;
}