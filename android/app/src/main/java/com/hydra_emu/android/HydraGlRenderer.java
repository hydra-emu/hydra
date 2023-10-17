package com.hydra_emu.android;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES20;
import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class HydraGlRenderer implements GLSurfaceView.Renderer {

    int screenTexture;
    public int screenFbo;

    private int shaderProgram;
    private int positionAttribute;
    private int texCoordAttribute;
    private int quadVertexBuffer;
    public static int loadShader(int type, String shaderCode) {
        int shader = GLES32.glCreateShader(type);
        GLES32.glShaderSource(shader, shaderCode);
        GLES32.glCompileShader(shader);
        final int[] compileStatus = new int[1];
        GLES32.glGetShaderiv(shader, GLES32.GL_COMPILE_STATUS, compileStatus, 0);

        if (compileStatus[0] == 0) {
            String infoLog = GLES32.glGetShaderInfoLog(shader);
            GLES32.glDeleteShader(shader);
            throw new RuntimeException("Shader compilation failed:\n" + infoLog);
        }

        return shader;
    }
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        Log.w("hydra", GLES32.glGetString(GLES32.GL_EXTENSIONS));
        Log.w("hydra", GLES32.glGetString(GLES32.GL_VERSION));
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        int[] generateBuffer = new int[1];
        GLES32.glGenTextures(1, generateBuffer, 0);
        screenTexture = generateBuffer[0];
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, screenTexture);
        GLES32.glTexStorage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_RGBA8, 400, 480);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_NEAREST);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_NEAREST);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
        GLES32.glGenFramebuffers(1, generateBuffer, 0);
        screenFbo = generateBuffer[0];
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, screenFbo);
        GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER, GLES32.GL_COLOR_ATTACHMENT0, GLES32.GL_TEXTURE_2D, screenTexture, 0);
        GLES32.glClearColor(1, 1, 0, 1);
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);

        String vertexShaderCode =
                "#version 310 es\n"+
                "layout(location = 0) in vec2 pos;\n"+
                "layout(location = 1) in vec2 uv;\n"+
                "out vec2 frag_uv;\n"+
                "void main(){\n"+
                    "gl_Position = vec4(pos, 0.0, 1.0);\n"+
                    "frag_uv = uv;\n"+
                "}\n";

        String fragmentShaderCode =
            "#version 310 es\n"+
            "precision highp float;\n" +
            "layout(location = 0) out vec4 color;\n"+
            "uniform sampler2D tex;\n"+
            "in vec2 frag_uv;\n"+
            "void main(){\n"+
            "    color = texture(tex, frag_uv);\n"+
            "}\n";
        int vertexShader = loadShader(GLES32.GL_VERTEX_SHADER, vertexShaderCode);
        int fragmentShader = loadShader(GLES32.GL_FRAGMENT_SHADER, fragmentShaderCode);
        shaderProgram = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgram, vertexShader);
        GLES32.glAttachShader(shaderProgram, fragmentShader);
        GLES32.glLinkProgram(shaderProgram);
        int[] linkStatus = new int[1];
        GLES32.glGetProgramiv(shaderProgram, GLES32.GL_LINK_STATUS, linkStatus, 0);
        if (linkStatus[0] != GLES32.GL_TRUE) {
            Log.e("TAG", "Shader program linking failed.");
        }
        float[] quadVertices = {
                // Positions (x, y), Texture coordinates (s, t)
                -1,  1, 0, 1,
                1,  1, 1, 1,
                1, -1, 1, 0,
                1, -1, 1, 0,
                -1, -1, 0, 0,
                -1,  1, 0, 1,
        };
        FloatBuffer vertexBuffer = ByteBuffer.allocateDirect(quadVertices.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        vertexBuffer.put(quadVertices).position(0);
        /*int[] vbo = new int[1];
        GLES32.glGenBuffers(1, vbo, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexBuffer.capacity() * 4, vertexBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glEnableVertexAttribArray(positionAttribute);
        GLES32.glEnableVertexAttribArray(texCoordAttribute);
        GLES32.glVertexAttribPointer(positionAttribute, 2, GLES32.GL_FLOAT, false, 16, 0);
        GLES32.glVertexAttribPointer(texCoordAttribute, 2, GLES32.GL_FLOAT, false, 16, 8);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);*/
        int[] vbo = new int[1];
        GLES32.glGenBuffers(1, vbo, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, vertexBuffer.capacity() * 4, vertexBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glEnableVertexAttribArray(0);
        GLES32.glVertexAttribPointer(0, 2, GLES32.GL_FLOAT, false, 4 * 4, 0);
        GLES32.glEnableVertexAttribArray(1);
        GLES32.glVertexAttribPointer(1, 2, GLES32.GL_FLOAT, false, 4 * 4, 2 * 4);
        int texLocation = GLES32.glGetUniformLocation(shaderProgram, "tex");
        GLES32.glUniform1i(texLocation, 0);
    }

    public void onDrawFrame(GL10 unused) {
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, screenFbo);
        GLES32.glClearColor(1,1,1,1);
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
        //GLES32.glUseProgram(shaderProgram);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, screenTexture);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 6);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
        GLES32.glUseProgram(0);
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        GLES32.glViewport(0, 0, width, height);
    }
}
