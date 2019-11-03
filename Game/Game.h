// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include <map>
#include <vector>
#include "GameDefs.h"
#include "StateProcessor.h"
#include "ProgressManager.h"
#include "GraphicsSettings.h"
#include "ScoresManager.h"

union SDL_Event;

namespace Chewman
{

class Game
{
public:
    static Game* getInstance();

    void update(float deltaTime);
    void setState(GameState newState);
    GameState getState() const;

    void processInput(const SDL_Event& event);

    void registerStateProcessor(GameState state, std::shared_ptr<StateProcessor> stateProcessor);

    ProgressManager& getProgressManager();
    GraphicsManager& getGraphicsManager();
    ScoresManager& getScoresManager();

private:
    Game() = default;
    void initStates();

private:
    static std::unique_ptr<Game> _instance;

    GameState _gameState = GameState::MainMenu;
    ProgressManager _progressManager;
    GraphicsManager _graphicsManager;
    ScoresManager _scoresManager;
    std::map<GameState, std::shared_ptr<StateProcessor>> _stateProcessors;

    std::vector<GameState> _overlappedStateList;
};

} // namespace Chewman