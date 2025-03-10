package com.example.extra;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;
import java.io.File;
import androidx.core.app.ActivityCompat;
import android.content.pm.PackageManager;
import android.Manifest;
import android.widget.Button;
import android.view.View;
import android.content.Intent;
import android.net.Uri;
import android.content.Intent;
import android.net.Uri;
import android.provider.MediaStore;
import android.database.Cursor;
import android.os.Build;
import android.media.MediaScannerConnection;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import androidx.annotation.NonNull;
import android.provider.Settings;
import androidx.annotation.Nullable;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE;
    
    static {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            PERMISSIONS_STORAGE = new String[]{
                Manifest.permission.READ_MEDIA_VIDEO
            };
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            PERMISSIONS_STORAGE = new String[]{
                Manifest.permission.MANAGE_EXTERNAL_STORAGE
            };
        } else {
            PERMISSIONS_STORAGE = new String[]{
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            };
        }
    }

    static {
        try {
            System.loadLibrary("extra");
            Log.i("FFmpegDecoder", "keyword: native库加载成功");
        } catch (UnsatisfiedLinkError e) {
            Log.e("FFmpegDecoder", "keyword: native库加载失败", e);
        }
    }

    private native int decodeToYUV(String inputPath, String outputPath);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv = findViewById(R.id.sample_text);
        Button requestPermissionButton = findViewById(R.id.request_permission_button);
        
        // 检查权限
        if (checkPermissions()) {
            tv.setText("准备开始解码...");
            startDecode();
        } else {
            tv.setText("需要存储权限");
            requestPermissionButton.setVisibility(View.VISIBLE);
            requestPermissionButton.setOnClickListener(v -> requestPermissions());
        }
    }

    private boolean checkPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }

    private void requestPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.addCategory("android.intent.category.DEFAULT");
                intent.setData(Uri.parse(String.format("package:%s", getApplicationContext().getPackageName())));
                startActivityForResult(intent, REQUEST_EXTERNAL_STORAGE);
            } catch (Exception e) {
                Intent intent = new Intent();
                intent.setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                startActivityForResult(intent, REQUEST_EXTERNAL_STORAGE);
            }
        } else {
            ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_EXTERNAL_STORAGE) {
            if (checkPermissions()) {
                TextView tv = findViewById(R.id.sample_text);
                tv.setText("准备开始解码...");
                startDecode();
            }
        }
    }

    private void startDecode() {
        try {
            if (!checkPermissions()) {
                TextView tv = findViewById(R.id.sample_text);
                tv.setText("需要存储权限");
                Log.e("FFmpegDecoder", "keyword: 权限检查失败");
                return;
            }

            // 输出权限状态
            Log.i("FFmpegDecoder", "keyword: READ_EXTERNAL_STORAGE权限状态: " + 
                (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED));
            
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Log.i("FFmpegDecoder", "keyword: MANAGE_EXTERNAL_STORAGE权限状态: " + 
                    Environment.isExternalStorageManager());
            }

            // 验证文件是否可读
            File downloadDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            File inputFile = new File(downloadDir, "test.mp4");
            Log.i("FFmpegDecoder", "keyword: 输入文件是否存在: " + inputFile.exists());
            Log.i("FFmpegDecoder", "keyword: 输入文件是否可读: " + inputFile.canRead());
            
            if (inputFile.exists()) {
                // 准备输出文件
                File outputFile = new File(getExternalFilesDir(null), "output.yuv");
                String outputPath = outputFile.getAbsolutePath();
                String inputPath = inputFile.getAbsolutePath();

                Log.i("FFmpegDecoder", "keyword: 找到输入文件: " + inputPath);
                
                // 输出文件路径信息
                Log.i("FFmpegDecoder", "keyword: 输出文件路径: " + outputPath);
                Log.i("FFmpegDecoder", "keyword: 应用私有目录: " + getExternalFilesDir(null).getAbsolutePath());
                Log.i("FFmpegDecoder", "keyword: 内部存储目录: " + getFilesDir().getAbsolutePath());

                // 开始解码
                new Thread(() -> {
                    try {
                        Log.i("FFmpegDecoder", "keyword: 开始调用native解码方法");
                        int result = decodeToYUV(inputPath, outputPath);
                        Log.i("FFmpegDecoder", "keyword: native解码方法返回值: " + result);
                        
                        runOnUiThread(() -> {
                            TextView tv = findViewById(R.id.sample_text);
                            if (result == 0) {
                                if (outputFile.exists()) {
                                    String message = "解码成功，输出文件大小：" + outputFile.length() + " 字节";
                                    Log.i("FFmpegDecoder", "keyword: " + message);
                                    tv.setText(message);
                                } else {
                                    String message = "解码完成，但输出文件不存在";
                                    Log.e("FFmpegDecoder", "keyword: " + message);
                                    tv.setText(message);
                                }
                            } else {
                                String message = "解码失败，错误码：" + result;
                                Log.e("FFmpegDecoder", "keyword: " + message);
                                tv.setText(message);
                            }
                        });
                    } catch (Exception e) {
                        Log.e("FFmpegDecoder", "keyword: 解码过程发生异常", e);
                        runOnUiThread(() -> {
                            TextView tv = findViewById(R.id.sample_text);
                            tv.setText("解码异常：" + e.getMessage());
                        });
                    }
                }).start();
            } else {
                TextView tv = findViewById(R.id.sample_text);
                tv.setText("在Download目录未找到test.mp4文件");
                Log.e("FFmpegDecoder", "keyword: 文件不存在: " + inputFile.getAbsolutePath());
            }
            
        } catch (Exception e) {
            Log.e("FFmpegDecoder", "keyword: startDecode发生异常", e);
            TextView tv = findViewById(R.id.sample_text);
            tv.setText("启动解码失败：" + e.getMessage());
        }
    }
}