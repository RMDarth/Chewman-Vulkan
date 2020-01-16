// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include "IGameMapService.h"

namespace Chewman
{

struct PlayerInfo
{
    uint32_t lives = 2;
    uint32_t points = 0;
    uint32_t time = 0;
};

struct LevelInfo
{
    uint16_t timeFor2Stars = 0;
    uint16_t timeFor3Stars = 0;
    std::string levelName;
};

class ProgressManager
{
public:
    ProgressManager();

    uint32_t getCurrentLevel() const;
    void setCurrentLevel(uint32_t level);

    const LevelInfo& getCurrentLevelInfo() const;
    void setCurrentLevelInfo(const LevelInfo& levelInfo);

    uint32_t getCurrentWorld() const;
    void setCurrentWorld(uint32_t world);

    IGameMapService* getGameMapService();
    void setGameMapService(IGameMapService* service);

    bool isStarted() const;
    void setStarted(bool started);

    bool isVictory() const;
    void setVictory(bool value);

    PlayerInfo& getPlayerInfo();
    void resetPlayerInfo();

private:
    uint32_t _currentWorld = 0;
    uint32_t _currentLevel = 0;
    bool _isStarted = false;
    bool _isVictory = false;

    PlayerInfo _playerInfo;
    LevelInfo _levelInfo;
    IGameMapService* _gameMapService = nullptr;
};

} // namespace Chewman