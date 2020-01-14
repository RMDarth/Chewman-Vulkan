// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Game/SoundSystem.h"
#include <SDL2/SDL.h>
#include <jni.h>
#include <string>

namespace Chewman
{

class AndroidSound : public Sound
{
public:
    AndroidSound(int id, bool loop) : _id(id), _loop(loop)
    {
    }

    void play() override
    {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

        jobject activity = (jobject)SDL_AndroidGetActivity();

        jclass activityClass = env->GetObjectClass(activity);

        jmethodID methodID = env->GetMethodID(activityClass, "playSound", "(IFZ)V");

        env->CallVoidMethod(activity, methodID, _id, _volume, _loop);

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);
    }

    void stop() override
    {
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

        jobject activity = (jobject)SDL_AndroidGetActivity();

        jclass activityClass = env->GetObjectClass(activity);

        jmethodID methodID = env->GetMethodID(activityClass, "stopSound", "(I)V");

        env->CallVoidMethod(activity, methodID, _id);

        env->DeleteLocalRef(activity);
        env->DeleteLocalRef(activityClass);
    }

    void setVolume(float volume) override
    {
        _volume = volume;
        if (_id == -100) // music
        {
            JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

            jobject activity = (jobject)SDL_AndroidGetActivity();

            jclass activityClass = env->GetObjectClass(activity);

            jmethodID methodID = env->GetMethodID(activityClass, "setMusicVolume", "(F)V");

            env->CallVoidMethod(activity, methodID, _volume);

            env->DeleteLocalRef(activity);
            env->DeleteLocalRef(activityClass);
        }
    }

private:
    int _id = -1;
    bool _loop = false;
    float _volume = 1.0f;
};

SoundSystem* SoundSystem::_instance = nullptr;

SoundSystem::SoundSystem() = default;
SoundSystem::~SoundSystem() = default;

SoundSystem* SoundSystem::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new SoundSystem();
    }
    return _instance;
}

bool SoundSystem::isLoaded() const
{
    return true;
}

std::shared_ptr<Sound> SoundSystem::createSound(const std::string& filename, bool loop)
{
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

    jobject activity = (jobject)SDL_AndroidGetActivity();

    jclass activityClass = env->GetObjectClass(activity);

    // Get the ID of the method we want to call
    // This must match the name and signature from the Java side
    // Signature has to match java implementation (second string hints a t a java string parameter)
    jmethodID methodID = env->GetMethodID(activityClass, "loadSound", "(Ljava/lang/String;Z)I");

    // Strings passed to the function need to be converted to a java string object
    jstring jfilename = env->NewStringUTF(filename.c_str());

    int id = env->CallIntMethod(activity, methodID, jfilename, loop);

    // Remember to clean up passed values
    env->DeleteLocalRef(jfilename);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activityClass);

    return std::make_shared<AndroidSound>(id, loop);
}

void SoundSystem::initBackgroundMusic(const std::string& filename)
{
    _bgm = createSound(filename, true);
}

void SoundSystem::startBackgroundMusic()
{
    if (_bgm != nullptr && !_isBgmPlaying)
    {
        _bgm->play();
        _isBgmPlaying = true;
    }
}

void SoundSystem::stopBackgroundMusic()
{
    if (_bgm != nullptr)
    {
        _bgm->stop();
        _isBgmPlaying = false;
    }
}

void SoundSystem::setBackgroundMusicVolume(float volume)
{
    if (_bgm != nullptr)
        _bgm->setVolume(volume);
}

} // namespace Chewman

