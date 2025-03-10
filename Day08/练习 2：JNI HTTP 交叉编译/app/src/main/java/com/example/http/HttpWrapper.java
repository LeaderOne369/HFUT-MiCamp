package com.example.http;

public class HttpWrapper {
    static {
        System.loadLibrary("httplib");
    }

    public native String sendGetRequest(String url);
} 