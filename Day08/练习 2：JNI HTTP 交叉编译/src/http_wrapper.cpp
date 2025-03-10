#include <jni.h>
#include <string>
#include <curl/curl.h>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_http_HttpWrapper_sendGetRequest(
        JNIEnv *env,
        jobject /* this */,
        jstring url) {
    
    const char *nativeUrl = env->GetStringUTFChars(url, 0);
    
    CURL *curl = curl_easy_init();
    std::string response_string;
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, nativeUrl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    
    env->ReleaseStringUTFChars(url, nativeUrl);
    
    return env->NewStringUTF(response_string.c_str());
} 