#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

#define LOG_TAG "NativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 触摸波纹结构体
struct TouchRipple
{
    float x, y;
    float age;
    float strength;

    TouchRipple(float _x, float _y) : x(_x), y(_y), age(0), strength(1.0f) {}
};

// 全局变量
static std::atomic<bool> g_running(false);
static std::vector<TouchRipple> g_ripples;
static std::mutex g_rippleMutex;
static float g_touchX = -1;
static float g_touchY = -1;

// 停止渲染循环
extern "C" JNIEXPORT void JNICALL
Java_com_example_anativewindowdemo_MainActivity_stopRendering(JNIEnv *env, jobject thiz)
{
    g_running = false;
}

// 处理触摸事件
extern "C" JNIEXPORT void JNICALL
Java_com_example_anativewindowdemo_MainActivity_onTouch(JNIEnv *env, jobject thiz, jfloat x, jfloat y)
{
    std::lock_guard<std::mutex> lock(g_rippleMutex);
    g_touchX = x;
    g_touchY = y;

    // 如果是有效的触摸点，添加新的波纹
    if (x >= 0 && y >= 0)
    {
        g_ripples.emplace_back(x, y);
        // 限制波纹数量，避免性能问题
        if (g_ripples.size() > 20)
        {
            g_ripples.erase(g_ripples.begin());
        }
    }
}

// 主渲染函数
extern "C" JNIEXPORT void JNICALL
Java_com_example_anativewindowdemo_MainActivity_setSurface(
    JNIEnv *env, jobject thiz, jobject surface, jint width, jint height)
{

    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr)
    {
        LOGE("无法获取原生窗口");
        return;
    }

    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    g_running = true;

    // 创建动画循环
    while (g_running)
    {
        // 计算帧时间
        auto currentTime = std::chrono::high_resolution_clock::now();
        float totalTime = std::chrono::duration<float>(currentTime - startTime).count();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        lastFrameTime = currentTime;

        ANativeWindow_Buffer buffer;
        if (ANativeWindow_lock(window, &buffer, nullptr) != 0)
        {
            LOGE("无法锁定窗口");
            break;
        }

        uint32_t *pixels = static_cast<uint32_t *>(buffer.bits);
        int stride = buffer.stride;

        // 更新波纹状态
        {
            std::lock_guard<std::mutex> lock(g_rippleMutex);

            // 更新现有波纹
            for (auto it = g_ripples.begin(); it != g_ripples.end();)
            {
                it->age += deltaTime;
                it->strength = 1.0f - std::min(1.0f, it->age / 3.0f);

                if (it->strength <= 0)
                {
                    it = g_ripples.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        // 性能优化：使用多个工作线程并行处理渲染
        const int numThreads = 4; // 线程数量
        std::vector<std::thread> workers;

        for (int t = 0; t < numThreads; ++t)
        {
            workers.push_back(std::thread([&, t]()
                                          {
                // 计算每个线程处理的行范围
                int startY = (buffer.height * t) / numThreads;
                int endY = (buffer.height * (t + 1)) / numThreads;
                
                for (int y = startY; y < endY; ++y) {
                    for (int x = 0; x < buffer.width; ++x) {
                        // 背景波纹效果
                        float dx = x - buffer.width / 2;
                        float dy = y - buffer.height / 2;
                        float distance = std::sqrt(dx * dx + dy * dy);
                        float wave = std::sin(distance * 0.03 - totalTime * 2.0) * 0.3 + 0.7;
                        
                        // 基础颜色
                        uint8_t r = static_cast<uint8_t>(std::sin(totalTime * 0.5 + x * 0.005) * 60 + 120);
                        uint8_t g = static_cast<uint8_t>(std::sin(totalTime * 0.5 + y * 0.005) * 60 + 120);
                        uint8_t b = static_cast<uint8_t>(std::sin(totalTime * 0.5 + distance * 0.005) * 60 + 120);
                        
                        // 应用波纹效果
                        float totalEffect = 0.0f;
                        float maxEffect = 0.0f;
                        
                        {
                            std::lock_guard<std::mutex> lock(g_rippleMutex);
                            for (const auto& ripple : g_ripples) {
                                float rippleDx = x - ripple.x;
                                float rippleDy = y - ripple.y;
                                float rippleDist = std::sqrt(rippleDx * rippleDx + rippleDy * rippleDy);
                                
                                // 波纹扩散效果
                                float rippleWidth = 200.0f;
                                float rippleSpeed = 300.0f;
                                float progress = (rippleSpeed * ripple.age) / rippleWidth;
                                float ringRadius = rippleSpeed * ripple.age;
                                
                                // 环形波纹
                                float ringEffect = std::exp(-(rippleDist - ringRadius) * (rippleDist - ringRadius) / (2 * rippleWidth * rippleWidth)) 
                                                   * ripple.strength;
                                               
                                // 累积波纹效果
                                totalEffect += ringEffect;
                                maxEffect = std::max(maxEffect, ringEffect);
                            }
                        }
                        
                        // 应用波纹效果到颜色
                        if (totalEffect > 0) {
                            // 增加波纹亮度
                            r = static_cast<uint8_t>(std::min(255.0f, r * (1.0f + totalEffect)));
                            g = static_cast<uint8_t>(std::min(255.0f, g * (1.0f + totalEffect)));
                            b = static_cast<uint8_t>(std::min(255.0f, b * (1.0f + totalEffect)));
                            
                            // 波纹颜色覆盖
                            if (maxEffect > 0.7f) {
                                float blend = (maxEffect - 0.7f) / 0.3f;
                                r = static_cast<uint8_t>(r * (1.0f - blend) + 255 * blend);
                                g = static_cast<uint8_t>(g * (1.0f - blend) + 255 * blend);
                                b = static_cast<uint8_t>(b * (1.0f - blend) + 240 * blend);
                            }
                        }
                        
                        r = static_cast<uint8_t>(r * wave);
                        g = static_cast<uint8_t>(g * wave);
                        b = static_cast<uint8_t>(b * wave);
                        
                        // 设置像素颜色
                        pixels[y * stride + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                } }));
        }

        for (auto &worker : workers)
        {
            worker.join();
        }

        ANativeWindow_unlockAndPost(window);

        // 性能优化：自适应帧率
        float targetFrameTime = 1.0f / 60.0f;
        float sleepTime = targetFrameTime - deltaTime;
        if (sleepTime > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
        }
    }

    ANativeWindow_release(window);
    LOGI("渲染线程已停止");
}