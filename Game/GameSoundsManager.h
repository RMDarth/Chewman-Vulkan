// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <map>
#include <memory>

namespace Chewman
{

class Sound;

enum class SoundType : uint8_t
{
    ChewCoin,
    ChewWall,
    ChewEnemy,
    Bomb,
    GargoyleStart,
    MagicTeleport,
    MagicFireball,
    PowerUp,
    PowerDown,
    Death,
    Victory,
    Star1,
    Star2,
    Star3
};

class GameSoundsManager
{
public:
    GameSoundsManager();
    GameSoundsManager(const GameSoundsManager&) = delete;

    void playSound(SoundType type);

    void setSoundVolume(float value);
    void setMusicVolume(float value);
    float getSoundVolume() const;
    float getMusicVolume() const;

    void setSoundEnabled(bool value);
    void setMusicEnabled(bool value);
    void pauseMusic();
    void unpauseMusic();
    bool isSoundEnabled() const;
    bool isMusicEnabled() const;

    void save();
    void load();

private:
    std::map<SoundType, std::shared_ptr<Sound>> _soundMap;

    float _soundVolume = 1.0f;
    float _musicVolume = 0.4f;
    bool _soundEnabled = true;
    bool _musicEnabled = true;
};

} // namespace Chewman