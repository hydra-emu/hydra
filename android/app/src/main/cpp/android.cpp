#include <android/log.h>
#include <core_loader.hxx>
#include <cstdio>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <filesystem>
#include <hydra/core.hxx>
#include <jni.h>
#include <string>

std::unique_ptr<hydra::EmulatorWrapper> emulator;

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

extern "C" JNIEXPORT void JNICALL Java_com_hydra_1emu_android_HydraGlRenderer_loadCore(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jstring path)
{
    emulator = hydra::EmulatorFactory::Create(jstring2string(env, path));
    if (!emulator)
    {
        __android_log_print(ANDROID_LOG_ERROR, "hydra", "Failed to load core");
        exit(1);
    }

    auto shell_gl = emulator->shell->getIOpenGlRendered();
    if (shell_gl)
    {
        shell_gl->setContext(eglGetCurrentContext());
        shell_gl->setGetProcAddress((void*)eglGetProcAddress);
    }
    else
    {
        __android_log_print(ANDROID_LOG_ERROR, "hydra", "dynamic_cast failed");
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_hydra_1emu_android_HydraGlRenderer_runFrame(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jint fbo)
{
    emulator->shell->getIOpenGlRendered()->setFbo(fbo);
    emulator->shell->getIFrontendDriven()->runFrame();
}

extern "C" JNIEXPORT void JNICALL Java_com_hydra_1emu_android_HydraGlRenderer_loadGame(JNIEnv* env,
                                                                                       jobject thiz,
                                                                                       jstring path)
{
    auto cpath = jstring2string(env, path);
    emulator->shell->loadFile("rom", cpath.c_str());
}
