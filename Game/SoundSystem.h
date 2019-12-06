// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>

namespace Chewman
{

class Sound
{
public:
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
};

class SoundSystem
{
public:
    ~SoundSystem();

    static SoundSystem* getInstance();

    bool isLoaded() const;
    std::shared_ptr<Sound> createSound(const std::string& filename, bool loop = false);

    void initBackgroundMusic(const std::string& filename);
    void startBackgroundMusic();
    void stopBackgroundMusic();
    void setBackgroundMusicVolume(float volume);

private:
    static SoundSystem* _instance;
    std::shared_ptr<Sound> _bgm;
    bool _isBgmPlaying = false;

    SoundSystem();
};

} // namespace Chewman