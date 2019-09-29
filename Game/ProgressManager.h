// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>

namespace Chewman
{

struct PlayerInfo
{
    uint32_t lives = 2;
    uint32_t points = 0;
};

class ProgressManager
{
public:
    ProgressManager();

    uint32_t getCurrentLevel();
    void setCurrentLevel(uint32_t level);

    bool isStarted();
    void setStarted(bool started);

    bool isVictory();
    void setVictory(bool value);

    PlayerInfo& getPlayerInfo();
    void resetPlayerInfo();

private:
    uint32_t _currentLevel = 1;
    bool _isStarted = false;
    bool _isVictory = false;

    PlayerInfo _playerInfo;
};

} // namespace Chewman