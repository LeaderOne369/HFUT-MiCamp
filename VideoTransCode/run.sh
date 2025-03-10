#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 检测操作系统
OS=$(uname)

if [ "$OS" = "Darwin" ]; then
    # macOS系统设置库路径
    export DYLD_LIBRARY_PATH="$SCRIPT_DIR/build/lib:$DYLD_LIBRARY_PATH"
elif [ "$OS" = "Linux" ]; then
    # 检测是否为Ubuntu
    if [ -f /etc/lsb-release ] && grep -q "Ubuntu" /etc/lsb-release; then
        # Ubuntu系统设置库路径
        export LD_LIBRARY_PATH="$SCRIPT_DIR/build/lib:$LD_LIBRARY_PATH"
    else
        echo "本程序仅支持macOS和Ubuntu。"
        exit 1
    fi
else
    echo "本程序仅支持macOS和Ubuntu。"
    exit 1
fi

# 获取速度参数
SPEED="$1"

# 设置默认值
INPUT_FILE="$SCRIPT_DIR/resource/1.mp4"
OUTPUT_DIR="$SCRIPT_DIR/resource"
SPEED_SUFFIX=""
SPEED_VALUE="2" # 默认2倍速

# 处理速度参数
if [ -z "$SPEED" ]; then
    echo "未指定速度参数，默认使用2倍速"
else
    case "$SPEED" in
        0.5)
            SPEED_SUFFIX="_0.5"
            SPEED_VALUE="0"
            echo "使用0.5倍速"
            ;;
        1|1.0)
            SPEED_SUFFIX="_1.0"
            SPEED_VALUE="1"
            echo "使用1.0倍速"
            ;;
        2|2.0)
            SPEED_SUFFIX="_2.0"
            SPEED_VALUE="2"
            echo "使用2.0倍速"
            ;;
        3|3.0)
            SPEED_SUFFIX="_3.0"
            SPEED_VALUE="3"
            echo "使用3.0倍速"
            ;;
        *)
            echo "不支持的速度参数: $SPEED，使用默认2倍速"
            ;;
    esac
fi

# 设置输出文件路径
VIDEO_OUTPUT="$OUTPUT_DIR/speed$SPEED_SUFFIX.mp4"
AUDIO_OUTPUT="$OUTPUT_DIR/speed$SPEED_SUFFIX.ac3"
FINAL_OUTPUT="$OUTPUT_DIR/mi$SPEED_SUFFIX.mp4"

echo "===== 开始处理视频 ====="
echo "输入文件: $INPUT_FILE"
echo "视频输出: $VIDEO_OUTPUT"
echo "音频输出: $AUDIO_OUTPUT"
echo "最终输出: $FINAL_OUTPUT"
echo ""

# 第一步：处理视频
echo "第一步：处理视频..."
"$SCRIPT_DIR/build/video_transcode" "$INPUT_FILE" "$VIDEO_OUTPUT" "$SPEED_VALUE"
if [ $? -ne 0 ]; then
    echo "视频处理失败！"
    exit 1
fi
echo "视频处理完成！"
echo ""

# 第二步：处理音频
echo "第二步：处理音频..."
"$SCRIPT_DIR/build/audio_transcode" "$INPUT_FILE" "$OUTPUT_DIR$SPEED_SUFFIX" "$SPEED_VALUE"
if [ $? -ne 0 ]; then
    echo "音频处理失败！"
    exit 1
fi
echo "音频处理完成！"
echo ""

# 第三步：合并视频和音频
echo "第三步：合并视频和音频..."
"$SCRIPT_DIR/build/muxer_execute" "$VIDEO_OUTPUT" "$AUDIO_OUTPUT" "$FINAL_OUTPUT"
if [ $? -ne 0 ]; then
    echo "视频音频合并失败！"
    exit 1
fi
echo "视频音频合并完成！"
echo ""

echo "===== 处理完成 ====="
echo "最终输出文件: $FINAL_OUTPUT"
echo ""
echo "您可以使用以下命令播放生成的文件："
echo "ffplay $FINAL_OUTPUT"
echo ""
echo "或者直接在resource目录中找到 mi$SPEED_SUFFIX.mp4 文件播放" 