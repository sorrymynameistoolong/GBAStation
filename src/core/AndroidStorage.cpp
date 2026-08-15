#include "core/AndroidStorage.hpp"

#ifdef __ANDROID__
#include <SDL_system.h>
#include <jni.h>

#include <borealis.hpp>
#endif

namespace beiklive::android_storage
{

bool requestRomImport()
{
#ifdef __ANDROID__
    auto* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !activity)
    {
        brls::Logger::error("Android document picker is unavailable because the SDL activity is not ready");
        return false;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (!activityClass)
    {
        env->DeleteLocalRef(activity);
        brls::Logger::error("Could not resolve the Android activity class for ROM import");
        return false;
    }

    const jmethodID importMethod = env->GetMethodID(
        activityClass, "importRomsFromSystemPicker", "()V");
    if (!importMethod)
    {
        env->ExceptionClear();
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(activity);
        brls::Logger::error("Android activity does not expose the ROM import method");
        return false;
    }

    env->CallVoidMethod(activity, importMethod);
    const bool succeeded = !env->ExceptionCheck();
    if (!succeeded)
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        brls::Logger::error("Android system document picker could not be opened");
    }

    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
    return succeeded;
#else
    return false;
#endif
}

} // namespace beiklive::android_storage
