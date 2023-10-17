#include <EGL/egl.h>
#include <jni.h>

EGLContext gl_context = 0;
unsigned fbo = 0;
using func_ptr = void (*)();
using get_proc_address_ptr = func_ptr (*)(const char*);
get_proc_address_ptr get_proc_address = nullptr;

extern "C" JNIEXPORT jstring JNICALL
Java_com_hydra_1emu_android_HydraGlSurfaceView_getNativeString(JNIEnv* env, jobject jo)
{
    gl_context = eglGetCurrentContext();
    get_proc_address = eglGetProcAddress;
    return env->NewStringUTF("Whatever");
}