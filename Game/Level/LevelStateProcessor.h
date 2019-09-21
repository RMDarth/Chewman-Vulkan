// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Game/StateProcessor.h"
#include "GameMap.h"
#include "GameMapLoader.h"

namespace Chewman
{

class GameMapProcessor;
class ProgressManager;

class LevelStateProcessor : public StateProcessor
{
public:
    LevelStateProcessor();
    ~LevelStateProcessor() override;

    void initMap();

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

private:
    GameMapLoader _mapLoader;
    ProgressManager& _progressManager;
    std::unique_ptr<GameMapProcessor> _gameMapProcessor;

    // As prev game map could be still in some commands, we need to finish rendering them all before release
    // TODO: Fix this in Engine so it won't destroy until all commands are finished
    std::unique_ptr<GameMapProcessor> _oldGameMap;
    uint32_t _countToRemove = 0;
};

} // namespace Chewman