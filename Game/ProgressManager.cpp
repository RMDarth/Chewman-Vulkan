// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ProgressManager.h"

namespace Chewman
{

ProgressManager::ProgressManager()
{
}

uint32_t ProgressManager::getCurrentLevel()
{
    return _currentLevel;
}

void ProgressManager::setCurrentLevel(uint32_t level)
{
    _currentLevel = level;
}

bool ProgressManager::isStarted()
{
    return _isStarted;
}

void ProgressManager::setStarted(bool started)
{
    _isStarted = started;
}

bool ProgressManager::isVictory()
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

} // namespace Chewman