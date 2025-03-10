#ifndef MUXER_H
#define MUXER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

#include "videoencoder.h"

class Muxer
{
public:
    Muxer();
    ~Muxer();

    bool init(const std::string &outputFile, const std::string &format = "");
    bool addVideoStream(AVCodecContext *codecCtx);
    bool addAudioStream(AVCodecContext *codecCtx);
    bool writeHeader();
    bool writePacket(VideoPacket *packet);
    bool writeTrailer();
    void close();

    AVFormatContext *getFormatContext() const { return formatCtx_; }

private:
    AVFormatContext *formatCtx_ = nullptr;
    std::string outputFile_;
    std::string format_;

    std::vector<int> streamIndexMap_;

    bool isInitialized_ = false;
    bool isHeaderWritten_ = false;
};

#endif // MUXER_H