package com.hydra_emu.android;

import static android.Manifest.permission.MANAGE_EXTERNAL_STORAGE;
import static android.Manifest.permission.READ_EXTERNAL_STORAGE;
import static android.provider.Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION;
import static android.provider.Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.documentfile.provider.DocumentFile;

import android.Manifest;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileUtils;
import android.provider.Settings;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.google.android.material.floatingactionbutton.FloatingActionButton;

import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class MainActivity extends AppCompatActivity {

    HydraGlSurfaceView glView;
    private ActivityResultLauncher<Intent> filePickerLauncher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!Environment.isExternalStorageManager()) {
            Intent intent = new Intent(ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
            startActivity(intent);
        }
        if (!Environment.isExternalStorageManager()) {
            Log.e("hydra", "Not enough permissions");
            return;
        }
        Resources resources = getResources();
        InputStream isv = resources.openRawResource(R.raw.vshader);
        InputStream isf = resources.openRawResource(R.raw.fshader);
        String vert, frag;
        try {
            vert = IOUtils.toString(isv, StandardCharsets.UTF_8);
            frag = IOUtils.toString(isf, StandardCharsets.UTF_8);
            isv.close();
            isf.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        glView = new HydraGlSurfaceView(this);
        setContentView(glView);

        FloatingActionButton fab = new FloatingActionButton(this);
        fab.setUseCompatPadding(true);
        fab.setOnClickListener(view -> fabClick());
        FrameLayout.LayoutParams fabLayoutParams = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT);
        fabLayoutParams.gravity = Gravity.BOTTOM | Gravity.END;
        fab.setLayoutParams(fabLayoutParams);
        fab.setImageResource(android.R.drawable.ic_menu_edit);
        addContentView(fab, fab.getLayoutParams());
        filePickerLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(),
                new ActivityResultCallback<ActivityResult>() {
                    @Override
                    public void onActivityResult(ActivityResult result) {
                        if (result.getResultCode() == AppCompatActivity.RESULT_OK) {
                            Intent data = result.getData();
                            if (data != null) {
                                Uri uri = data.getData();

                            }
                        }
                    }
                });
    }

    private void fabClick() {
        glView.queueEvent(new Runnable() {
            @Override
            public void run() {
                //Intent act = new Intent(MainActivity.this, SettingsActivity.class);
                //MainActivity.this.startActivity(act);
                //openFilePicker();
                glView.queueEvent(new Runnable() {
                                      @Override
                                      public void run() {
                                          String core_path = getApplicationInfo().dataDir + "/cores/libAlber_x86_64.so";
                                          glView.loadCore(core_path);
                                          String game_path = getApplicationInfo().dataDir + "/games/zelda.3ds";
                                          glView.loadGame(game_path);
                                          for (int i = 0; i < 600; i++)
                                            glView.runFrame(glView.renderer.screenFbo);
                                          glView.requestRender();
                                      }
                                  });

            }
        });
    }

    private void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*"); // Allow all file types
        filePickerLauncher.launch(intent);
    }

    private byte[] readFileDataFromUri(@NonNull Uri uri) throws IOException {
        ContentResolver contentResolver = getContentResolver();
        InputStream inputStream = contentResolver.openInputStream(uri);
        byte[] data;
        if (inputStream != null) {
            data = new byte[inputStream.available()];
            inputStream.read(data);
            inputStream.close();
            return data;
        } else {
            throw new IOException("Failed to open input stream for the selected file.");
        }
    }

}