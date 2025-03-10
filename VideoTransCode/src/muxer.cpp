#include "muxer.h"
#include <iostream>
#include <libavutil/version.h>
#include <libavcodec/version.h>

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

Muxer::Muxer() {}

Muxer::~Muxer()
{
    close();
}

bool Muxer::init(const std::string &outputFile, const std::string &format)
{
    outputFile_ = outputFile;
    format_ = format;

    int ret = 0;
    if (!format.empty())
    {
        ret = avformat_alloc_output_context2(&formatCtx_, nullptr, format.c_str(), outputFile.c_str());
        if (ret < 0 || !formatCtx_)
        {
            cout << "无法创建指定格式的输出上下文: " << format << endl;
            return false;
        }
    }
    else
    {
        ret = avformat_alloc_output_context2(&formatCtx_, nullptr, nullptr, outputFile.c_str());
    }

    if (ret < 0 || !formatCtx_)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "无法创建输出上下文: " << errBuf << endl;
        return false;
    }

    if (!(formatCtx_->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&formatCtx_->pb, outputFile.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
            av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
            cout << "无法打开输出文件: " << errBuf << endl;
            return false;
        }
    }

    isInitialized_ = true;
    cout << "封装器初始化成功: " << outputFile << endl;
    return true;
}

bool Muxer::addVideoStream(AVCodecContext *codecCtx)
{
    if (!isInitialized_ || !codecCtx)
    {
        cout << "封装器未初始化或编码器上下文无效" << endl;
        return false;
    }

    AVStream *stream = avformat_new_stream(formatCtx_, nullptr);
    if (!stream)
    {
        cout << "无法创建视频流" << endl;
        return false;
    }

    stream->id = formatCtx_->nb_streams - 1;

    int ret = avcodec_parameters_from_context(stream->codecpar, codecCtx);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "无法复制编码器参数: " << errBuf << endl;
        return false;
    }

    stream->time_base = codecCtx->time_base;
    streamIndexMap_.push_back(stream->id);

    cout << "添加视频流成功: " << codecCtx->width << "x" << codecCtx->height << endl;
    return true;
}

bool Muxer::addAudioStream(AVCodecContext *codecCtx)
{
    if (!isInitialized_ || !codecCtx)
    {
        cout << "封装器未初始化或编码器上下文无效" << endl;
        return false;
    }

    AVStream *stream = avformat_new_stream(formatCtx_, nullptr);
    if (!stream)
    {
        cout << "无法创建音频流" << endl;
        return false;
    }

    stream->id = formatCtx_->nb_streams - 1;

    int ret = avcodec_parameters_from_context(stream->codecpar, codecCtx);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "无法复制编码器参数: " << errBuf << endl;
        return false;
    }

    stream->time_base = codecCtx->time_base;
    streamIndexMap_.push_back(stream->id);

    cout << "添加音频流成功: " << codecCtx->sample_rate << "Hz, "
         << get_channels(codecCtx) << "通道" << endl;
    return true;
}

bool Muxer::writeHeader()
{
    if (!isInitialized_ || !formatCtx_)
    {
        cout << "封装器未初始化" << endl;
        return false;
    }

    if (isHeaderWritten_)
    {
        cout << "头部已经写入" << endl;
        return true;
    }

    int ret = avformat_write_header(formatCtx_, nullptr);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "写入文件头失败: " << errBuf << endl;
        return false;
    }

    isHeaderWritten_ = true;
    cout << "写入文件头成功" << endl;
    return true;
}

bool Muxer::writePacket(VideoPacket *packet)
{
    if (!isInitialized_ || !formatCtx_ || !isHeaderWritten_)
    {
        cout << "封装器未初始化或头部未写入" << endl;
        return false;
    }

    if (!packet || !packet->packet)
    {
        cout << "无效的数据包" << endl;
        return false;
    }

    if (packet->streamIndex >= 0 && packet->streamIndex < streamIndexMap_.size())
    {
        packet->packet->stream_index = streamIndexMap_[packet->streamIndex];
    }
    else
    {
        cout << "无效的流索引: " << packet->streamIndex << endl;
        return false;
    }

    int ret = av_interleaved_write_frame(formatCtx_, packet->packet);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "写入数据包失败: " << errBuf << endl;
        return false;
    }

    packet->packet = nullptr;

    return true;
}

bool Muxer::writeTrailer()
{
    if (!isInitialized_ || !formatCtx_ || !isHeaderWritten_)
    {
        cout << "封装器未初始化或头部未写入" << endl;
        return false;
    }

    int ret = av_write_trailer(formatCtx_);
    if (ret < 0)
    {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        cout << "写入文件尾失败: " << errBuf << endl;
        return false;
    }

    cout << "写入文件尾成功" << endl;
    return true;
}

void Muxer::close()
{
    if (formatCtx_)
    {
        if (formatCtx_->pb && !(formatCtx_->oformat->flags & AVFMT_NOFILE))
        {
            avio_closep(&formatCtx_->pb);
        }

        avformat_free_context(formatCtx_);
        formatCtx_ = nullptr;
    }

    outputFile_.clear();
    format_.clear();
    streamIndexMap_.clear();
    isInitialized_ = false;
    isHeaderWritten_ = false;
}
