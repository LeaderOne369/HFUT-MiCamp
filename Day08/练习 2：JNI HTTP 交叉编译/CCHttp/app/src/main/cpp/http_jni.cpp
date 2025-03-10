#include <jni.h>
#include <string>
#include <android/log.h>
#include <fstream>
#include "libghttp/ghttp.h"
#include "libghttp/http_global.h"

#define LOG_TAG "HttpJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static JavaVM* g_jvm = nullptr;
static jclass g_httpClient_class = nullptr;
static jmethodID g_onResponse_method = nullptr;
static jmethodID g_onDownloadProgress_method = nullptr;
static jmethodID g_onDownloadComplete_method = nullptr;
static jmethodID g_onDownloadError_method = nullptr;

// JNI_OnLoad 函数，用于初始化
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // 获取 HttpClient 类的引用
    jclass localClass = env->FindClass("com/mi/cc/http/HttpClient");
    if (!localClass) {
        LOGE("Failed to find HttpClient class");
        return JNI_ERR;
    }
    g_httpClient_class = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
    
    // 获取方法 ID
    g_onResponse_method = env->GetMethodID(g_httpClient_class, "onResponse", "(Ljava/lang/String;)V");
    g_onDownloadProgress_method = env->GetMethodID(g_httpClient_class, "onDownloadProgress", "(II)V");
    g_onDownloadComplete_method = env->GetMethodID(g_httpClient_class, "onDownloadComplete", "(Ljava/lang/String;)V");
    g_onDownloadError_method = env->GetMethodID(g_httpClient_class, "onDownloadError", "(Ljava/lang/String;)V");

    if (!g_onResponse_method || !g_onDownloadProgress_method || 
        !g_onDownloadComplete_method || !g_onDownloadError_method) {
        LOGE("Failed to find methods");
        return JNI_ERR;
    }

    LOGI("JNI_OnLoad completed successfully");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_mi_cc_http_HttpClient_httpGet(JNIEnv* env, jobject thiz, jstring url) {
    const char* native_url = env->GetStringUTFChars(url, nullptr);
    
    LOGI("Starting HTTP GET request to: %s", native_url);
    
    // 检查 URL 是否是 HTTPS
    if (strncmp(native_url, "https://", 8) == 0) {
        LOGE("HTTPS protocol is not supported by libghttp, please use HTTP instead");
        jstring response = env->NewStringUTF("错误：libghttp 不支持 HTTPS 协议，请使用 HTTP");
        env->CallVoidMethod(thiz, g_onResponse_method, response);
        env->DeleteLocalRef(response);
        env->ReleaseStringUTFChars(url, native_url);
        return;
    }
    
    // 创建 HTTP 请求
    ghttp_request* request = ghttp_request_new();
    if (!request) {
        LOGE("Failed to create request");
        jstring response = env->NewStringUTF("错误：创建请求失败");
        env->CallVoidMethod(thiz, g_onResponse_method, response);
        env->DeleteLocalRef(response);
        env->ReleaseStringUTFChars(url, native_url);
        return;
    }

    // 设置 URI
    if (ghttp_set_uri(request, (char*)native_url) == -1) {
        LOGE("Failed to set URI: %s", native_url);
        jstring response = env->NewStringUTF("错误：设置 URL 失败");
        env->CallVoidMethod(thiz, g_onResponse_method, response);
        env->DeleteLocalRef(response);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        return;
    }

    // 设置请求类型为 GET
    ghttp_set_type(request, ghttp_type_get);

    // 设置请求头
    ghttp_set_header(request, http_hdr_Accept, "*/*");
    ghttp_set_header(request, http_hdr_Connection, "close");
    ghttp_set_header(request, "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    ghttp_set_header(request, "Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");

    // 准备请求
    if (ghttp_prepare(request) == -1) {
        LOGE("Failed to prepare request: %s", ghttp_get_error(request));
        jstring response = env->NewStringUTF("错误：准备请求失败");
        env->CallVoidMethod(thiz, g_onResponse_method, response);
        env->DeleteLocalRef(response);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        return;
    }

    // 发送请求并处理重定向
    int max_redirects = 5;
    int redirect_count = 0;
    ghttp_status status;
    const char* location;

    while (redirect_count < max_redirects) {
        status = ghttp_process(request);
        if (status == ghttp_error) {
            break;
        }

        int status_code = ghttp_status_code(request);
        LOGI("HTTP status code: %d", status_code);

        if (status_code == 301 || status_code == 302) {
            location = ghttp_get_header(request, http_hdr_Location);
            if (!location) {
                LOGE("Redirect without Location header");
                break;
            }

            LOGI("Following redirect to: %s", location);
            
            // 检查重定向 URL 是否是 HTTPS
            if (strncmp(location, "https://", 8) == 0) {
                LOGE("Redirect to HTTPS URL is not supported");
                jstring response = env->NewStringUTF("错误：重定向到 HTTPS URL 不支持");
                env->CallVoidMethod(thiz, g_onResponse_method, response);
                env->DeleteLocalRef(response);
                ghttp_request_destroy(request);
                env->ReleaseStringUTFChars(url, native_url);
                return;
            }

            // 创建新的请求对象
            ghttp_request* new_request = ghttp_request_new();
            if (!new_request) {
                break;
            }

            if (ghttp_set_uri(new_request, (char*)location) == -1) {
                ghttp_request_destroy(new_request);
                break;
            }

            ghttp_set_type(new_request, ghttp_type_get);
            ghttp_set_header(new_request, http_hdr_Accept, "*/*");
            ghttp_set_header(new_request, http_hdr_Connection, "close");
            ghttp_set_header(new_request, "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
            ghttp_set_header(new_request, "Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");

            if (ghttp_prepare(new_request) == -1) {
                ghttp_request_destroy(new_request);
                break;
            }

            ghttp_request_destroy(request);
            request = new_request;
            redirect_count++;
            continue;
        }

        // 非重定向状态，退出循环
        break;
    }

    // 检查最终状态
    if (status == ghttp_error || redirect_count >= max_redirects) {
        const char* error = ghttp_get_error(request);
        LOGE("Request failed: %s", error ? error : "Unknown error");
        std::string error_msg = "错误：请求失败";
        if (error) {
            error_msg += std::string(" - ") + error;
        }
        jstring response = env->NewStringUTF(error_msg.c_str());
        env->CallVoidMethod(thiz, g_onResponse_method, response);
        env->DeleteLocalRef(response);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        return;
    }

    // 获取响应
    int body_len = ghttp_get_body_len(request);
    LOGI("Response body length: %d", body_len);
    
    if (body_len > 0) {
        const char* body = ghttp_get_body(request);
        if (!body) {
            LOGE("Response body is null");
            jstring errorMsg = env->NewStringUTF("错误：响应内容为空");
            env->CallVoidMethod(thiz, g_onResponse_method, errorMsg);
            env->DeleteLocalRef(errorMsg);
        } else {
            // 创建一个新的缓冲区来存储响应内容
            std::string response_str;
            try {
                response_str.assign(body, body_len);
                jstring response = env->NewStringUTF(response_str.c_str());
                if (response) {
                    LOGI("Calling Java callback with response");
                    env->CallVoidMethod(thiz, g_onResponse_method, response);
                    env->DeleteLocalRef(response);
                } else {
                    LOGE("Failed to create response string");
                    jstring errorMsg = env->NewStringUTF("错误：无法创建响应字符串");
                    env->CallVoidMethod(thiz, g_onResponse_method, errorMsg);
                    env->DeleteLocalRef(errorMsg);
                }
            } catch (const std::exception& e) {
                LOGE("Exception while handling response: %s", e.what());
                jstring errorMsg = env->NewStringUTF("错误：处理响应时发生异常");
                env->CallVoidMethod(thiz, g_onResponse_method, errorMsg);
                env->DeleteLocalRef(errorMsg);
            }
        }
    } else {
        LOGE("No response body received");
        const char* error = ghttp_get_error(request);
        if (error) {
            LOGE("Error message: %s", error);
            jstring errorMsg = env->NewStringUTF(error);
            env->CallVoidMethod(thiz, g_onResponse_method, errorMsg);
            env->DeleteLocalRef(errorMsg);
        }
        
        int status_code = ghttp_status_code(request);
        LOGE("HTTP status code: %d", status_code);
        std::string error_msg = "错误：HTTP 状态码 " + std::to_string(status_code);
        jstring errorMsg = env->NewStringUTF(error_msg.c_str());
        env->CallVoidMethod(thiz, g_onResponse_method, errorMsg);
        env->DeleteLocalRef(errorMsg);
    }

    // 清理资源
    ghttp_request_destroy(request);
    env->ReleaseStringUTFChars(url, native_url);
}

extern "C" JNIEXPORT void JNICALL
Java_com_mi_cc_http_HttpClient_httpDownload(JNIEnv* env, jobject thiz, jstring url, jstring save_path) {
    const char* native_url = env->GetStringUTFChars(url, nullptr);
    const char* native_save_path = env->GetStringUTFChars(save_path, nullptr);
    
    LOGI("Starting HTTP download from: %s", native_url);
    LOGI("Saving to: %s", native_save_path);

    // 检查 HTTPS
    if (strncmp(native_url, "https://", 8) == 0) {
        LOGE("HTTPS protocol is not supported by libghttp");
        jstring error = env->NewStringUTF("HTTPS protocol is not supported");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 创建请求对象
    ghttp_request* request = ghttp_request_new();
    if (!request) {
        LOGE("Failed to create request");
        jstring error = env->NewStringUTF("Failed to create HTTP request");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 设置 URI
    if (ghttp_set_uri(request, (char*)native_url) == -1) {
        LOGE("Failed to set URI: %s", native_url);
        jstring error = env->NewStringUTF("Failed to set URI");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 设置请求类型为 GET
    ghttp_set_type(request, ghttp_type_get);

    // 设置请求头
    ghttp_set_header(request, http_hdr_Accept, "*/*");
    ghttp_set_header(request, http_hdr_Connection, "close");
    ghttp_set_header(request, "User-Agent", "Mozilla/5.0");

    // 准备请求
    if (ghttp_prepare(request) == -1) {
        LOGE("Failed to prepare request: %s", ghttp_get_error(request));
        jstring error = env->NewStringUTF("Failed to prepare request");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 发送请求
    ghttp_status status = ghttp_process(request);
    if (status == ghttp_error) {
        LOGE("Failed to process request: %s", ghttp_get_error(request));
        jstring error = env->NewStringUTF("Failed to process request");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 获取响应体
    const char* body = ghttp_get_body(request);
    int body_len = ghttp_get_body_len(request);
    
    if (body_len <= 0 || !body) {
        LOGE("No data received");
        jstring error = env->NewStringUTF("No data received");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 更新进度（开始）
    env->CallVoidMethod(thiz, g_onDownloadProgress_method, 0, body_len);

    // 写入文件
    FILE* file = fopen(native_save_path, "wb");
    if (!file) {
        LOGE("Failed to open file for writing: %s", native_save_path);
        jstring error = env->NewStringUTF("Failed to open file for writing");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    size_t written = fwrite(body, 1, body_len, file);
    fclose(file);

    if (written != (size_t)body_len) {
        LOGE("Error writing to file: %zu of %d bytes written", written, body_len);
        jstring error = env->NewStringUTF("Error writing to file");
        env->CallVoidMethod(thiz, g_onDownloadError_method, error);
        env->DeleteLocalRef(error);
        ghttp_request_destroy(request);
        env->ReleaseStringUTFChars(url, native_url);
        env->ReleaseStringUTFChars(save_path, native_save_path);
        return;
    }

    // 更新进度（完成）
    env->CallVoidMethod(thiz, g_onDownloadProgress_method, body_len, body_len);

    // 下载完成
    LOGI("Download completed, saved to: %s", native_save_path);
    jstring path = env->NewStringUTF(native_save_path);
    env->CallVoidMethod(thiz, g_onDownloadComplete_method, path);
    env->DeleteLocalRef(path);

    // 清理资源
    ghttp_request_destroy(request);
    env->ReleaseStringUTFChars(url, native_url);
    env->ReleaseStringUTFChars(save_path, native_save_path);
} 