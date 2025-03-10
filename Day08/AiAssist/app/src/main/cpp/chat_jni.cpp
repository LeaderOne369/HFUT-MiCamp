#include <jni.h>
#include <string>
#include "cJSON/cJSON.h"
#include <android/log.h>

#define LOG_TAG "ChatJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_mi_ai_assist_MainActivity_encodeObjToJsonStr(JNIEnv *env, jobject thiz, jobject data) {
    LOGI("Starting encodeObjToJsonStr");
    
    jclass questionClass = env->GetObjectClass(data);
    if (!questionClass) {
        LOGE("Failed to get QuestionData class");
        return NULL;
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
    jstring result = env->NewStringUTF(jsonString);
    
    free(jsonString);
    cJSON_Delete(root);
    env->ReleaseStringUTFChars(questionStr, question);
    
    LOGI("encodeObjToJsonStr completed successfully");
    return result;
}

JNIEXPORT jobject JNICALL
Java_com_mi_ai_assist_MainActivity_decodeJsonStrToObj(JNIEnv *env, jobject thiz, jstring jsonStr) {
    LOGI("Starting decodeJsonStrToObj");
    const char *json = env->GetStringUTFChars(jsonStr, 0);
    LOGI("Input JSON: %s", json);
    
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        LOGE("Failed to parse JSON: %s", cJSON_GetErrorPtr());
        env->ReleaseStringUTFChars(jsonStr, json);
        return NULL;
    }
    
    cJSON *answer = cJSON_GetObjectItem(root, "answer");
    cJSON *time = cJSON_GetObjectItem(root, "time");
    
    if (!answer || !time) {
        LOGE("Missing required fields in JSON");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(jsonStr, json);
        return NULL;
    }
    
    LOGI("Parsed answer: %s", answer->valuestring);
    LOGI("Parsed time: %f", time->valuedouble);
    
    jclass answerClass = env->FindClass("com/mi/ai/assist/AnswerData");
    if (!answerClass) {
        LOGE("Failed to find AnswerData class");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(jsonStr, json);
        return NULL;
    }
    
    jmethodID constructor = env->GetMethodID(answerClass, "<init>", "()V");
    if (!constructor) {
        LOGE("Failed to get AnswerData constructor");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(jsonStr, json);
        return NULL;
    }
    
    jobject answerData = env->NewObject(answerClass, constructor);
    
    jfieldID answerField = env->GetFieldID(answerClass, "answer", "Ljava/lang/String;");
    jfieldID timeField = env->GetFieldID(answerClass, "time", "F");
    
    if (!answerField || !timeField) {
        LOGE("Failed to get field IDs");
        cJSON_Delete(root);
        env->ReleaseStringUTFChars(jsonStr, json);
        return NULL;
    }
    
    env->SetObjectField(answerData, answerField, env->NewStringUTF(answer->valuestring));
    env->SetFloatField(answerData, timeField, (jfloat)time->valuedouble);
    
    LOGI("Successfully created AnswerData object");
    
    cJSON_Delete(root);
    env->ReleaseStringUTFChars(jsonStr, json);
    
    return answerData;
}

} 