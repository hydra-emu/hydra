package com.hydra_emu.android;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES32.*;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class HydraGlRenderer implements GLSurfaceView.Renderer {

    int screenTexture;
    public int screenFbo;
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        Log.w("hydra", glGetString(GL_EXTENSIONS));
        Log.w("hydra", glGetString(GL_VERSION));
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        int[] generateBuffer = new int[1];
        glGenTextures(1, generateBuffer, 0);
        screenTexture = generateBuffer[0];
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 600, 600);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenFramebuffers(1, generateBuffer, 0);
        screenFbo = generateBuffer[0];
        glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    public void onDrawFrame(GL10 unused) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFbo);
        glBlitFramebuffer(0, 0, 600, 600, 0, 0, 600, 600, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        glViewport(0, 0, 600, 600);
        glDisable(GL_SCISSOR_TEST);
    }
}
