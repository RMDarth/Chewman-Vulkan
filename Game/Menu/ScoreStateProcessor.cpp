// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ScoreStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <iomanip>
#include <sstream>

namespace Chewman
{

namespace
{

std::string timeToString(uint32_t time)
{
    std::stringstream stream;
    stream << time / 60 << ":" << std::setfill('0') << std::setw(2) << time % 60;
    return stream.str();
}

} // anon namespace

ScoreStateProcessor::ScoreStateProcessor()
    : _progressManager(Game::getInstance()->getProgressManager())
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/scoremenu.xml");
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

ScoreStateProcessor::~ScoreStateProcessor() = default;

GameState ScoreStateProcessor::update(float deltaTime)
{
    _time += deltaTime;
    if (_time < 3.0f)
    {
        auto scoreText = std::to_string(static_cast<int>(_progressManager.getPlayerInfo().points * std::min(_time, 2.0f) * 0.5f));
        _document->getControlByName("score")->setText("Score: " + scoreText);
    } else
    {
        _document->getControlByName("score")->setText("Score: " + std::to_string(_progressManager.getPlayerInfo().points));
    }

    return GameState::Score;
}

void ScoreStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void ScoreStateProcessor::show()
{
    _document->show();
    _document->getControlByName("result")->setText(_progressManager.isVictory() ? "Victory" : "You lose");
    _document->getControlByName("levelname")->setText("Level " + std::to_string(_progressManager.getCurrentLevel()));
    _document->getControlByName("time")->setText("Time: " + timeToString(_progressManager.getPlayerInfo().time));
    _document->getControlByName("score")->setText("Score: 0");
    _time = 0;
}

void ScoreStateProcessor::hide()
{
    _document->hide();
}

bool ScoreStateProcessor::isOverlapping()
{
    return true;
}

void ScoreStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "continue")
        {
            Game::getInstance()->setState(GameState::Level);
        }

        if (control->getName() == "restart")
        {
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            Game::getInstance()->setState(GameState::Level);
        }

        if (control->getName() == "tomenu")
        {
            _progressManager.setCurrentLevel(1);
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            Game::getInstance()->setState(GameState::MainMenu);
        }
    }
}

} // namespace Chewman