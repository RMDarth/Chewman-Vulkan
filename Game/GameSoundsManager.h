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

    void PlaySound(SoundType type);
private:
    std::map<SoundType, std::shared_ptr<Sound>> _soundMap;
};

} // namespace Chewman