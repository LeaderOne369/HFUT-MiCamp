#include <jni.h>
#include <string>
#include "cJSON/cJSON.h"
#include <android/log.h>

#define LOG_TAG "AiAssist"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C"
JNIEXPORT jstring JNICALL
Java_com_mi_ai_assist_MainActivity_encodeObjToJsonStr(JNIEnv *env, jobject thiz, jobject data) {
    jclass questionClass = env->GetObjectClass(data);
    if (!questionClass) {
        LOGE("Failed to get QuestionData class");
        return nullptr;
    }
    
    jfieldID questionField = env->GetFieldID(questionClass, "question", "Ljava/lang/String;");
    jfieldID timeField = env->GetFieldID(questionClass, "time", "J");
    
    jstring questionStr = (jstring)env->GetObjectField(data, questionField);
    jlong timestamp = env->GetLongField(data, timeField);
    
    const char *question = env->GetStringUTFChars(questionStr, 0);
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "question", question);
    cJSON_AddNumberToObject(root, "time", (double)timestamp);
    
    char *jsonString = cJSON_Print(root);
    LOGI("Generated JSON: %s", jsonString);
    jstring result = env->NewStringUTF(jsonString);
    
    free(jsonString);
    cJSON_Delete(root);
    env->ReleaseStringUTFChars(questionStr, question);
    
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_mi_ai_assist_MainActivity_decodeJsonStrToObj(JNIEnv *env, jobject thiz, jstring json_str) {
    const char *json = env->GetStringUTFChars(json_str, 0);
    LOGI("Parsing JSON response: %s", json);
    
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        LOGE("Failed to parse JSON: %s", cJSON_GetErrorPtr());
        env->ReleaseStringUTFChars(json_str, json);
        return nullptr;
    }
    
    cJSON *answer = cJSON_GetObjectItem(root, "answer");
    cJSON *time = cJSON_GetObjectItem(root, "time");
    
    if (!answer || !time) {
        LOGE("Missing required fields in JSON");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(json_str, json);
        return nullptr;
    }
    
    jclass answerClass = env->FindClass("com/mi/ai/assist/AnswerData");
    if (!answerClass) {
        LOGE("Failed to find AnswerData class");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(json_str, json);
        return nullptr;
    }
    
    jmethodID constructor = env->GetMethodID(answerClass, "<init>", "()V");
    jobject answerData = env->NewObject(answerClass, constructor);
    
    jfieldID answerField = env->GetFieldID(answerClass, "answer", "Ljava/lang/String;");
    jfieldID timeField = env->GetFieldID(answerClass, "time", "F");
    
    env->SetObjectField(answerData, answerField, env->NewStringUTF(answer->valuestring));
    env->SetFloatField(answerData, timeField, (jfloat)time->valuedouble);
    
    cJSON_Delete(root);
    env->ReleaseStringUTFChars(json_str, json);
    
    return answerData;
}