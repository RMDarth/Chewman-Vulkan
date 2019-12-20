// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SystemApi.h"
#include <SDL2/SDL.h>
#ifdef __ANDROID__
#include <jni.h>
#endif

namespace Chewman
{
namespace System
{

int getSystemVersion()
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "getAndroidVersion", "()I");

    int version = env->CallIntMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return version;
#endif
    return 0;
}

bool acceptQuary(const std::string& message, const std::string& title, const std::string& accept, const std::string& decline)
{
    const SDL_MessageBoxButtonData buttons[] = {
            { 0, 0, accept.c_str()},
            { 0, 1, decline.c_str()}
    };

    const SDL_MessageBoxData messageBoxData = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            nullptr, /* .window */
            title.c_str(), /* .title */
            message.c_str(), /* .message */
            SDL_arraysize(buttons), /* .numbuttons */
            buttons, /* .buttons */
            nullptr
    };
    int buttonId;
    if (SDL_ShowMessageBox(&messageBoxData, &buttonId) < 0) {
        SDL_Log("Error displaying message box");
        return false;
    }

    return !(buttonId == -1 || buttonId == 1);
}

void restartApp()
{
#ifdef __ANDROID__
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass activityClass = env->GetObjectClass(activity);

    jmethodID methodId = env->GetMethodID(activityClass, "restartApp", "()V");

    env->CallVoidMethod(activity, methodId);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);
#else
    // TODO: Add Desktop restart functionality
    exit(0);
#endif
}

void exitApp()
{
    SDL_Quit();
    exit(0);
}

} // namespace System
} // namespace Chewman