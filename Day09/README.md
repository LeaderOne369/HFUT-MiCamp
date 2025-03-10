# Day09：音视频

## 课堂作业

1. 将 MP4 文件转成 FLV 和 MKV 格式，之后使用 `ffprobe` 查看这三个视频。
2. 在 Ubuntu 上编译 FFmpeg 源码，完成开源项目的自主安装。
3. 使用 FFmpeg 的 `ffprobe` 命令，解析一个音视频，逐帧输出音视频文件的数据，并以 CSV 文件格式展示出来。
4. 利用 FFmpeg API 编写程序，将 `1.ts` 视频文件解析到 `AVPacket`，并打印其中数据包的 DTS 和 PTS 信息。
5. 利用 FFmpeg API 编写程序，将 `1.ts` 视频文件解码到 `AVFrame`（运用编解码信息进行编解码）。
6. 利用 FFmpeg API 编写程序，将 `1.ts` 视频文件保存成 YUV 格式视频。

## 课后作业

编译一个 Android 下 ARMv8 或 ARMv7 的 FFmpeg 库。

## 附加加分题作业（已实现）

### FFmpeg && Android

1. 编写一个 FFmpeg 在 Android 上的 APK，完成将视频解析到 YUV。
2. 将 YUV 保存代码在 Android 上实现。
3. 包括编译 FFmpeg 到 Android 平台，新建 Android 项目导入 FFmpeg 库，并移植上面 YUV 代码到 Android 工程。
