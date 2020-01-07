// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ProgressManager.h"

namespace Chewman
{

ProgressManager::ProgressManager() = default;

uint32_t ProgressManager::getCurrentLevel() const
{
    return _currentLevel;
}

void ProgressManager::setCurrentLevel(uint32_t level)
{
    _currentLevel = level;
}

uint32_t ProgressManager::getCurrentWorld() const
{
    return _currentWorld;
}

void ProgressManager::setCurrentWorld(uint32_t world)
{
    _currentWorld = world;
}

bool ProgressManager::isStarted() const
{
    return _isStarted;
}

void ProgressManager::setStarted(bool started)
{
    _isStarted = started;
}

bool ProgressManager::isVictory() const
{
    return _isVictory;
}

void ProgressManager::setVictory(bool value)
{
    _isVictory = value;
}

PlayerInfo& ProgressManager::getPlayerInfo()
{
    return _playerInfo;
}

void ProgressManager::resetPlayerInfo()
{
    _playerInfo = {};
}

const LevelInfo& ProgressManager::getCurrentLevelInfo() const
{
    return _levelInfo;
}

void ProgressManager::setCurrentLevelInfo(const LevelInfo& levelInfo)
{
    _levelInfo = levelInfo;
}

} // namespace Chewman