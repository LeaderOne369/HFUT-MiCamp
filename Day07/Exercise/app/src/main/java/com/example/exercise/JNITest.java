package com.example.exercise;

import android.util.Log;

public class JNITest {
    private static final String TAG = "练习 4-JNITest";
    
    static {
        System.loadLibrary("exercise");
    }
    public native int add(int a, int b);
    
    public static class HttpManager {
        private static final String TAG = "练习 4-HttpManager";
        
        public static void sendResultToServer(int result) {
            Log.d(TAG, "【Native调用Java】sendResultToServer: 接收到C++传递的结果: " + result);
            Log.i(TAG, "【练习 4】模拟结果发送成功，计算结果为: " + result);
        }
    }
} 