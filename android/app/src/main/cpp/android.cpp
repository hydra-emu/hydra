#include <jni.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_hydra_1emu_android_MainActivity_getNativeString(JNIEnv* env, jobject obj)
{
    return env->NewStringUTF("Hello World! From native code!");
}

int main()
{
    return 0;
}