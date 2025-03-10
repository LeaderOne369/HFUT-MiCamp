package com.example.assignment;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.example.assignment.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    // Used to load the 'assignment' library on application startup.
    static {
        System.loadLibrary("assignment");
    }

    private ActivityMainBinding binding;
    private EditText inputEditText;
    private TextView resultTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        inputEditText = binding.inputEditText;
        resultTextView = binding.resultTextView;
        Button reverseButton = binding.reverseButton;

        reverseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String inputText = inputEditText.getText().toString();
                if (inputText.isEmpty()) {
                    Toast.makeText(MainActivity.this, "请输入文本", Toast.LENGTH_SHORT).show();
                    return;
                }
                
                String result = reverseStringJNI(inputText);
                
                Log.d(TAG, result);
                
                resultTextView.setText(result);
            }
        });

        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }

    /**
     * A native method that is implemented by the 'assignment' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    
    /**
     * @param inputString
     * @return
     */
    public native String reverseStringJNI(String inputString);
}