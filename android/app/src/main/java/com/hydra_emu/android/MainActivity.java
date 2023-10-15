package com.hydra_emu.android;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    HydraGlSurfaceView glView;
    TextView textView;
    Button button;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // TextView textView = (TextView) findViewById(R.id.text_view);
        // textView.setText(getNativeString());
        glView = new HydraGlSurfaceView(this);
        setContentView(glView);
        textView = new TextView(this);
        textView.setText("YourText");
        textView.setTextColor(Color.WHITE);
        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 32);
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.CENTER;
        addContentView(textView, params);

        button = new Button(this);
        button.setText("Mybutton");
        button.setOnClickListener(
                v -> glView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        MainActivity.this.glView.renderer.setGreen();
                        MainActivity.this.glView.invalidate();
                    }
                })
        );
        FrameLayout.LayoutParams params1 = new FrameLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
        params1.gravity = Gravity.CENTER;
        addContentView(button, params1);
    }
}