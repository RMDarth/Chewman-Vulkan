// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <sstream>
#include "LevelStateProcessor.h"
#include "Game/Game.h"

namespace Chewman
{

LevelStateProcessor::LevelStateProcessor()
    : _progressManager(Game::getInstance()->getProgressManager())
{
}

LevelStateProcessor::~LevelStateProcessor()
{
}

void LevelStateProcessor::initMap()
{
    auto levelNum = _progressManager.getCurrentLevel();

    if (_gameMapProcessor)
    {
        _oldGameMap = std::move(_gameMapProcessor);
        _oldGameMap->setVisible(false);
        _countToRemove = 5;
    }

    std::stringstream ss;
    ss << "resources/game/levels/level" << levelNum << ".map";
    _gameMapProcessor = std::make_unique<Chewman::GameMapProcessor>(_mapLoader.loadMap(ss.str()));
}

GameState LevelStateProcessor::update(float deltaTime)
{
    _gameMapProcessor->update(deltaTime);

    switch (_gameMapProcessor->getState())
    {
        case GameMapState::Game:
            break;
        case GameMapState::Pause:
            break;
        case GameMapState::Animation:
            break;
        case GameMapState::Victory:
            // TODO: Display victory menu
            _progressManager.setCurrentLevel(_progressManager.getCurrentLevel() + 1);
            _progressManager.setVictory(false);
            initMap();
            break;
        case GameMapState::GameOver:
            // TODO: Display gameover menu
            _progressManager.setCurrentLevel(1);
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            return GameState::MainMenu;
    }

    if (_countToRemove > 0)
    {
        --_countToRemove;
        if (!_countToRemove)
            _oldGameMap.reset();
    }

    return GameState::Level;
}

void LevelStateProcessor::processInput(const SDL_Event& event)
{
    _gameMapProcessor->processInput(event);
}

void LevelStateProcessor::show()
{
    if (!_progressManager.isStarted())
    {
        _progressManager.setStarted(true);
        _progressManager.setVictory(false);
        initMap();
    }
}

void LevelStateProcessor::hide()
{
    _gameMapProcessor->setVisible(false);
}

bool LevelStateProcessor::isOverlapping()
{
    return false;
}

} // namespace Chewman