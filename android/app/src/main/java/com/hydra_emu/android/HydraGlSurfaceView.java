package com.hydra_emu.android;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.DisplayMetrics;

import com.hydra_emu.android.HydraGlRenderer;

public class HydraGlSurfaceView extends GLSurfaceView {
    final HydraGlRenderer renderer;

    public HydraGlSurfaceView(Context context) {
        super(context);
        setEGLContextClientVersion(3);
        renderer = new HydraGlRenderer();
        setRenderer(renderer);
    }

    static {
        System.loadLibrary("hydra_android");
    }
}