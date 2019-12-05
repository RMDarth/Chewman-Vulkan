// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Game.h"

#include <utility>
#include <Game/Menu/GraphicsStateProcessor.h>
#include "Game/Level/LevelStateProcessor.h"
#include "Game/Level/GameMapLoader.h"
#include "Game/Menu/MenuStateProcessor.h"
#include "Game/Menu/PauseStateProcessor.h"
#include "Game/Menu/ScoreStateProcessor.h"
#include "Game/Menu/WorldSelectionStateProcessor.h"
#include "Game/Menu/LevelSelectionStateProcessor.h"
#include "Game/Menu/GraphicsStateProcessor.h"
#include "Game/Menu/TutorialStateProcessor.h"
#include "Game/Menu/HighScoresStateProcessor.h"
#include "Game/Menu/CreditsStateProcessor.h"
#include "Game/Menu/SettingsStateProcessor.h"


namespace Chewman
{
std::unique_ptr<Game> Game::_instance = {};

Game* Game::getInstance()
{
    if (!_instance)
    {
        _instance = std::unique_ptr<Game>(new Game());
        _instance->initStates();
    }
    return _instance.get();
}

void Game::update(float deltaTime)
{
    auto newState = _stateProcessors[_gameState]->update(deltaTime);
    if (newState != _gameState)
        setState(newState);
}

void Game::setState(GameState newState)
{
    auto& newStateProcessor = _stateProcessors[newState];
    if (!newStateProcessor->isOverlapping())
    {
        _stateProcessors[_gameState]->hide();
        for (auto state : _overlappedStateList)
            _stateProcessors[state]->hide();
        _overlappedStateList.clear();
    } else {
        if (!_overlappedStateList.empty() && _overlappedStateList.back() == newState)
        {
            _overlappedStateList.pop_back();
        } else {
            _overlappedStateList.push_back(_gameState);
        }
    }

    _gameState = newState;
    newStateProcessor->show();
}

GameState Game::getState() const
{
    return _gameState;
}

void Game::processInput(const SDL_Event& event)
{
    _stateProcessors[_gameState]->processInput(event);
}

void Game::registerStateProcessor(GameState state, std::shared_ptr<StateProcessor> stateProcessor)
{
    _stateProcessors[state] = std::move(stateProcessor);
}

ProgressManager& Game::getProgressManager()
{
    return _progressManager;
}

GraphicsManager& Game::getGraphicsManager()
{
    return _graphicsManager;
}

ScoresManager& Game::getScoresManager()
{
    return _scoresManager;
}

GameMapLoader& Game::getGameMapLoader()
{
    return *_mapLoader;
}

GameSoundsManager& Game::getSoundsManager()
{
    return _soundsManager;
}

std::vector<std::string>& Game::getTutorialData()
{
    return _tutorialText;
}

Game::Game()
    : _mapLoader(std::make_unique<GameMapLoader>())
    , _graphicsManager(GraphicsManager::getInstance())
{
}

void Game::initStates()
{
    registerStateProcessor(GameState::Level, std::make_shared<LevelStateProcessor>());
    registerStateProcessor(GameState::MainMenu, std::make_shared<MenuStateProcessor>());
    registerStateProcessor(GameState::Pause, std::make_shared<PauseStateProcessor>());
    registerStateProcessor(GameState::Score, std::make_shared<ScoreStateProcessor>());
    registerStateProcessor(GameState::WorldSelection, std::make_shared<WorldSelectionStateProcessor>());
    registerStateProcessor(GameState::LevelSelection, std::make_shared<LevelSelectionStateProcessor>());
    registerStateProcessor(GameState::Graphics, std::make_shared<GraphicsStateProcessor>());
    registerStateProcessor(GameState::Tutorial, std::make_shared<TutorialStateProcessor>());
    registerStateProcessor(GameState::Highscores, std::make_shared<HighscoresStateProcessor>());
    registerStateProcessor(GameState::Credits, std::make_shared<CreditsStateProcessor>());
    registerStateProcessor(GameState::Settings, std::make_shared<SettingsStateProcessor>());

    _stateProcessors[_gameState]->show();
}

} // namespace Chewman