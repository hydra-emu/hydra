#include <android/log.h>
#include <core/core.h>
#include <cstdio>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <filesystem>
#include <jni.h>
#include <string>

EGLContext gl_context = 0;
unsigned gl_fbo = 0;
using func_ptr = void (*)();
using get_proc_address_ptr = func_ptr (*)(const char*);
get_proc_address_ptr get_proc_address = nullptr;
void* dlhandle = nullptr;
void* emu = nullptr;

#define X(symbol) decltype(symbol)* symbol##_p = nullptr;
HC_SYMBOLS
#undef X

std::string jstring2string(JNIEnv* env, jstring jStr)
{
    if (!jStr)
        return "";

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes =
        (jbyteArray)env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t)env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char*)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

void* get_other(hc_other_e other)
{
    switch (other)
    {
        case HC_OTHER_GL_CONTEXT:
        {
            return nullptr;
        }
        case HC_OTHER_GL_GET_PROC_ADDRESS:
        {
            return (void*)get_proc_address;
        }
        case HC_OTHER_GL_FBO:
        {
            return &gl_fbo;
        }
    }
}

void video_callback(const uint8_t*, uint32_t, uint32_t) {}

void poll_callback(){};

int8_t input_callback(uint8_t, hc_input_e)
{
    return 0;
};

extern "C" JNIEXPORT void JNICALL
Java_com_hydra_1emu_android_HydraGlSurfaceView_loadCore(JNIEnv* env, jobject thiz, jstring path)
{
    if (dlhandle)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "hydra", "Core already open");
        return;
    }

    get_proc_address = eglGetProcAddress;

    auto pathc = jstring2string(env, path);
    dlhandle = dlopen(pathc.c_str(), RTLD_LAZY);
    if (!dlhandle)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "hydra", "Could not dlopen core %s", dlerror());
        return;
    }
#define X(symbol) symbol##_p = (decltype(symbol)*)dlsym(dlhandle, #symbol);
    HC_SYMBOLS
#undef X
    hc_set_read_other_callback_p(get_other);
    emu = hc_create_p();
    hc_set_video_callback_p(emu, video_callback);
    hc_set_poll_input_callback_p(emu, poll_callback);
    hc_set_read_input_callback_p(emu, input_callback);
    if (!emu)
    {
        __android_log_print(ANDROID_LOG_INFO, "hydra", "Could not create emu!");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_hydra_1emu_android_HydraGlSurfaceView_runFrame(JNIEnv* env, jobject thiz, jint fbo)
{
    gl_fbo = fbo;
    hc_run_frame_p(emu);
}

extern "C" JNIEXPORT void JNICALL
Java_com_hydra_1emu_android_HydraGlSurfaceView_loadGame(JNIEnv* env, jobject thiz, jstring path)
{
    auto cpath = jstring2string(env, path);
    hc_load_file_p(emu, "rom", cpath.c_str());
}