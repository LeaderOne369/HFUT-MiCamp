#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "练习 4-Native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_exercise_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */)
{
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_exercise_JNITest_add(
    JNIEnv *env,
    jobject thiz,
    jint a,
    jint b)
{
    jint result = a + b;
    LOGD("【练习 4】C++计算: %d + %d = %d", a, b, result);

    LOGD("【练习 4】开始调用Java方法sendResultToServer");
    jclass httpManagerClass = env->FindClass("com/example/exercise/JNITest$HttpManager");
    if (httpManagerClass == NULL)
    {
        LOGI("【练习 4】错误: 无法找到HttpManager类");
        return result;
    }

    jmethodID sendResultMethod = env->GetStaticMethodID(httpManagerClass, "sendResultToServer", "(I)V");
    if (sendResultMethod == NULL)
    {
        LOGI("【练习 4】错误: 无法找到sendResultToServer方法");
        return result;
    }

    LOGD("【练习 4】调用Java方法: sendResultToServer(%d)", result);
    env->CallStaticVoidMethod(httpManagerClass, sendResultMethod, result);
    LOGI("【练习 4】成功将结果通过JNI传递给Java方法");

    return result;
}