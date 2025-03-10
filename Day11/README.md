# Day11：图形显示：OpenGL、DRM 介绍

今日作业较少，因此不再在各题的目录中分别撰写详细的 readme 文档，而是汇集在本文档中。

## 1. Libdrm 探索

### 题目描述

- 源代码：Android/external/Libdrm。
- 基于 libdrm 开发个应用显示简单的界面。

### 实现过程

#### 一、确保环境准备好了

- 打开终端：

```bash
sudo apt install build-essential libx11-dev
```

- 这里：
  - build-essential 用于编译 C、C++程序。
  - libx11-dev 提供 X11 窗口开发所需的头文件和库。

#### 二、编译老师给的代码：

- 执行：

```bash
g++ 1.cpp -o test -lX11
```

- 1.cpp 是老师给的源代码文件。
- -lX11 链接 X11 库。

#### 三、运行生成的程序：

- 在终端执行：

```bash
./test
```

- 执行后会弹出一个白色窗口，如图。

![alt text](resource/image-2.png)

- 这样整个作业在 Ubuntu 下就顺利完成了！

## 2. ANativeWindow 使用探索

### 题目描述

- ANativeWindow  是 Android 的 native 窗口。
- 直接基于  native Surface 相关控件实现简单的显示效果的应用。

### 运行结果

- 应用启动后会显示动态波纹背景：

![alt text](resource/image.png)

- 触摸屏幕任意位置产生波纹效果，波纹会从触摸点向外扩散，并逐渐消失：

![alt text](resource/image-1.png)

### 实现过程

#### 功能特点

- 利用 ANativeWindow 直接操作像素数据
- 实现动态彩色波纹动画效果
- 交互式触摸波纹效果，触摸屏幕任意位置产生扩散波纹
- 多线程渲染和自适应帧率优化
- 使用原生 C++代码实现高性能图形处理
- 展示了 Java 层与 C++层的交互方式

#### 实现原理

1. Java 层通过 SurfaceView 创建 Surface 并获取其句柄
2. 捕获触摸事件并通过 JNI 传递给 C++层
3. 使用 JNI 将 Surface 引用传递给 C++层
4. C++层使用 ANativeWindow_fromSurface 获取原生窗口引用
5. 通过 ANativeWindow_lock 获取像素缓冲区
6. 直接操作像素数据，实现波纹效果和颜色渐变
7. 使用多线程并行计算像素值，提高渲染效率
8. 使用 ANativeWindow_unlockAndPost 提交更改，显示在屏幕上

#### 代码结构

- `MainActivity.java`: 创建 SurfaceView 并处理触摸事件和生命周期
- `native-lib.cpp`: 包含原生 C++代码，实现波纹效果和多线程渲染
- `CMakeLists.txt`: CMake 构建脚本，配置优化选项
- `activity_main.xml`: 主界面布局
