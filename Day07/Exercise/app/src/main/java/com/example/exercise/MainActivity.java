package com.example.exercise;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.example.exercise.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "练习 4-MainActivity";

    static {
        System.loadLibrary("exercise");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        Log.d(TAG, "【练习 4】开始测试C++调用Java方法");
        JNITest jniTest = new JNITest();
        Log.d(TAG, "【练习 4】Java调用native方法: add(5, 3)");
        int result = jniTest.add(5, 3);
        Log.d(TAG, "【练习 4】Java收到native计算结果：" + result);
        TextView tv = binding.sampleText;
        tv.setText("【练习 4】原生方法返回：" + stringFromJNI() + "\nJNI add方法返回：" + result);
    }

    /**
     * A native method that is implemented by the 'exercise' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}