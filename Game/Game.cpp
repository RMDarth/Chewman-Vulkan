// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Game.h"

#include <utility>
#include "Game/Level/LevelStateProcessor.h"
#include "Game/Menu/MenuStateProcessor.h"

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

    // TODO: Add support for overlapping states
    _stateProcessors[_gameState]->hide();

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

Game::Game()
{
}

void Game::initStates()
{
    registerStateProcessor(GameState::Level, std::make_shared<LevelStateProcessor>());
    registerStateProcessor(GameState::MainMenu, std::make_shared<MenuStateProcessor>());

    _stateProcessors[_gameState]->show();
}

} // namespace Chewman