// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameSoundsManager.h"
#include "SoundSystem.h"

namespace Chewman
{

GameSoundsManager::GameSoundsManager()
{
    auto* soundSystem = SoundSystem::getInstance();
    if (SoundSystem::getInstance()->isLoaded())
    {
        _soundMap[SoundType::ChewCoin] = soundSystem->createSound("resources/sounds/chew2.ogg");

        soundSystem->initBackgroundMusic("resources/sounds/music.ogg");
        soundSystem->startBackgroundMusic();
    }
}

void GameSoundsManager::PlaySound(SoundType type)
{
    if (SoundSystem::getInstance()->isLoaded())
        _soundMap.at(type)->Play();
}

} // namespace Chewman