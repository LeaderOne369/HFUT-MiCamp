package com.mi.cc.http;

public class HttpClient {
    static {
        System.loadLibrary("cchttp");
    }

    private OnResponseCallback mCallback;
    private OnDownloadCallback mDownloadCallback;

    public interface OnResponseCallback {
        void onReceive(String response);
    }

    public interface OnDownloadCallback {
        void onProgress(int progress, int total);
        void onComplete(String filePath);
        void onError(String error);
    }

    public void setCallback(OnResponseCallback callback) {
        mCallback = callback;
    }

    public void setDownloadCallback(OnDownloadCallback callback) {
        mDownloadCallback = callback;
    }

    public native void httpGet(String url);
    public native void httpDownload(String url, String savePath);

    // 这些方法会被 C++ 代码调用
    private void onResponse(String response) {
        if (mCallback != null) {
            mCallback.onReceive(response);
        }
    }

    private void onDownloadProgress(int progress, int total) {
        if (mDownloadCallback != null) {
            mDownloadCallback.onProgress(progress, total);
        }
    }

    private void onDownloadComplete(String filePath) {
        if (mDownloadCallback != null) {
            mDownloadCallback.onComplete(filePath);
        }
    }

    private void onDownloadError(String error) {
        if (mDownloadCallback != null) {
            mDownloadCallback.onError(error);
        }
    }
} 