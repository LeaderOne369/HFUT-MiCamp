package com.mi.cc.http;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private HttpClient httpClient;
    private EditText urlInput;
    private TextView responseText;
    private ProgressBar progressBar;
    private Button getButton;
    private Button downloadButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        httpClient = new HttpClient();
        urlInput = findViewById(R.id.url_input);
        responseText = findViewById(R.id.response_text);
        progressBar = findViewById(R.id.progress_bar);
        getButton = findViewById(R.id.get_button);
        downloadButton = findViewById(R.id.download_button);

        // 设置默认URL
        urlInput.setText("http://www.example.com");

        // GET 请求回调
        httpClient.setCallback(response -> {
            Log.d(TAG, "Received response: " + response);
            runOnUiThread(() -> {
                responseText.setText(response);
                if (!response.startsWith("错误：")) {
                    Toast.makeText(MainActivity.this, "请求完成", Toast.LENGTH_SHORT).show();
                }
            });
        });

        // 下载回调
        httpClient.setDownloadCallback(new HttpClient.OnDownloadCallback() {
            @Override
            public void onProgress(int progress, int total) {
                runOnUiThread(() -> {
                    progressBar.setVisibility(View.VISIBLE);
                    progressBar.setMax(total);
                    progressBar.setProgress(progress);
                    responseText.setText(String.format("下载进度: %d/%d", progress, total));
                });
            }

            @Override
            public void onComplete(String filePath) {
                runOnUiThread(() -> {
                    progressBar.setVisibility(View.GONE);
                    responseText.setText("下载完成: " + filePath);
                    Toast.makeText(MainActivity.this, "下载完成", Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onError(String error) {
                runOnUiThread(() -> {
                    progressBar.setVisibility(View.GONE);
                    responseText.setText("下载错误: " + error);
                    Toast.makeText(MainActivity.this, "下载失败", Toast.LENGTH_SHORT).show();
                });
            }
        });

        // GET 请求按钮
        getButton.setOnClickListener(v -> {
            String url = urlInput.getText().toString().trim();
            if (url.isEmpty()) {
                Toast.makeText(this, "请输入URL", Toast.LENGTH_SHORT).show();
                return;
            }

            if (!url.startsWith("http://") && !url.startsWith("https://")) {
                url = "http://" + url;
                urlInput.setText(url);
            }

            Log.d(TAG, "Sending GET request to: " + url);
            responseText.setText("正在发送请求...");
            final String finalUrl = url;
            
            new Thread(() -> {
                try {
                    httpClient.httpGet(finalUrl);
                } catch (Exception e) {
                    Log.e(TAG, "Error during request", e);
                    runOnUiThread(() -> {
                        responseText.setText("请求出错: " + e.getMessage());
                        Toast.makeText(MainActivity.this, "请求失败", Toast.LENGTH_SHORT).show();
                    });
                }
            }).start();
        });

        // 下载按钮
        downloadButton.setOnClickListener(v -> {
            String url = urlInput.getText().toString().trim();
            if (url.isEmpty()) {
                Toast.makeText(this, "请输入URL", Toast.LENGTH_SHORT).show();
                return;
            }

            if (!url.startsWith("http://") && !url.startsWith("https://")) {
                url = "http://" + url;
                urlInput.setText(url);
            }

            // 使用应用专属目录
            File downloadDir = new File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), "CCHttp");
            if (!downloadDir.exists()) {
                downloadDir.mkdirs();
            }

            // 从 URL 中获取文件名
            String fileName = url.substring(url.lastIndexOf('/') + 1);
            if (fileName.isEmpty()) {
                fileName = "download.dat";
            }

            final String savePath = new File(downloadDir, fileName).getAbsolutePath();
            Log.d(TAG, "Starting download from: " + url);
            Log.d(TAG, "Saving to: " + savePath);

            responseText.setText("开始下载...");
            progressBar.setVisibility(View.VISIBLE);
            progressBar.setProgress(0);

            final String finalUrl = url;
            new Thread(() -> {
                try {
                    httpClient.httpDownload(finalUrl, savePath);
                } catch (Exception e) {
                    Log.e(TAG, "Error during download", e);
                    runOnUiThread(() -> {
                        responseText.setText("下载出错: " + e.getMessage());
                        progressBar.setVisibility(View.GONE);
                        Toast.makeText(MainActivity.this, "下载失败", Toast.LENGTH_SHORT).show();
                    });
                }
            }).start();
        });
    }
}
