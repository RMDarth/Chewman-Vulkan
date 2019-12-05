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
    virtual void Play() = 0;
    virtual void Stop() = 0;
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

private:
    static SoundSystem* _instance;
    std::shared_ptr<Sound> _bgm;

    SoundSystem();
};

} // namespace Chewman