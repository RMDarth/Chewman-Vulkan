// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <sstream>
#include <iomanip>
#include <future>
#include "LevelStateProcessor.h"
#include "Game/Game.h"
#include "Game/Controls/ControlDocument.h"

namespace Chewman
{

LevelStateProcessor::LevelStateProcessor()
    : _progressManager(Game::getInstance()->getProgressManager())
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/HUD.xml");
    _document->setMouseUpHandler(this);
    _document->hide();

    _loadingFinished = false;
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
    auto future = std::async(std::launch::async, [&]
    {
        _gameMapProcessor = std::make_unique<Chewman::GameMapProcessor>(_mapLoader.loadMap(ss.str()));
        _loadingFinished = true;
    });
}

GameState LevelStateProcessor::update(float deltaTime)
{
    if (_loadingFinished == false)
        return GameState::Level;

    _gameMapProcessor->update(deltaTime);
    _time += deltaTime;
    updateHUD();

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
            _progressManager.setVictory(true);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
        case GameMapState::GameOver:
            _progressManager.setCurrentLevel(1);
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
    }

    if (_countToRemove > 0)
    {
        --_countToRemove;
        if (!_countToRemove)
        {
            auto future = std::async(std::launch::async, [&]
            {
                _oldGameMap.reset();
            });
        }
    }

    return GameState::Level;
}

void LevelStateProcessor::processInput(const SDL_Event& event)
{
    _gameMapProcessor->processInput(event);
    processDocument(event, _document.get());
}

void LevelStateProcessor::show()
{
    if (!_progressManager.isStarted())
    {
        _progressManager.setStarted(true);
        _progressManager.setVictory(false);
        _loadingFinished = false;
        initMap();
        _time = 0.0f;
    } else {
        _gameMapProcessor->setState(GameMapState::Game);
    }
    _gameMapProcessor->setVisible(true);
    _document->show();
}

void LevelStateProcessor::hide()
{
    _gameMapProcessor->setVisible(false);
    _document->hide();
}

bool LevelStateProcessor::isOverlapping()
{
    return false;
}

void LevelStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "pause")
        {
            _gameMapProcessor->setState(GameMapState::Pause);
            Game::getInstance()->setState(GameState::Pause);
        }
    }
}

void LevelStateProcessor::updateHUD()
{
    std::stringstream stream;
    stream << (int)_time / 60 << ":" << std::setfill('0') << std::setw(2) << (int)_time % 60;
    static auto timeControl = _document->getControlByName("time");
    timeControl->setText(stream.str());

    static auto scoreControl = _document->getControlByName("score");
    scoreControl->setText(std::to_string(_progressManager.getPlayerInfo().points));

    static auto livesControl = _document->getControlByName("lifecount");
    livesControl->setText(std::to_string(_progressManager.getPlayerInfo().lives));

    static auto fps = _document->getControlByName("Coins");
    stream.str("");
    stream << _gameMapProcessor->getGameMap()->activeCoins << "/" << _gameMapProcessor->getGameMap()->totalCoins;
    fps->setText(stream.str());
    /*static uint32_t frames = 0;
    frames++;
    fps->setText(std::to_string((int)(frames/_time)));*/

}

} // namespace Chewman