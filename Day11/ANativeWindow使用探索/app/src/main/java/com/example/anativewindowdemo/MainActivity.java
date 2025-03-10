package com.example.anativewindowdemo;

import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private boolean running = false;
    private Thread renderThread;
    
    // 触摸点坐标
    private float touchX = -1;
    private float touchY = -1;

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surfaceView);
        surfaceHolder = surfaceView.getHolder();
        
        // 添加触摸事件监听
        surfaceView.setOnTouchListener((v, event) -> {
            int action = event.getAction();
            if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_MOVE) {
                touchX = event.getX();
                touchY = event.getY();
                // 将触摸位置传递给原生代码
                onTouch(touchX, touchY);
            } else if (action == MotionEvent.ACTION_UP) {
                touchX = -1;
                touchY = -1;
                onTouch(touchX, touchY);
            }
            return true;
        });

        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                final int width = holder.getSurfaceFrame().width();
                final int height = holder.getSurfaceFrame().height();
                
                running = true;
                renderThread = new Thread(() -> {
                    setSurface(holder.getSurface(), width, height);
                });
                renderThread.start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                running = false;
                stopRendering(); // 调用原生方法停止渲染循环
                if (renderThread != null) {
                    try {
                        renderThread.join(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }

    // 原生方法
    public native void setSurface(Surface surface, int width, int height);
    public native void onTouch(float x, float y);
    public native void stopRendering();
} 