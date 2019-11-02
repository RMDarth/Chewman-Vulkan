// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ScoreStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <iomanip>

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

    _restartX = _document->getControlByName("restart")->getPosition().x;
}

ScoreStateProcessor::~ScoreStateProcessor() = default;

GameState ScoreStateProcessor::update(float deltaTime)
{
    _time += deltaTime;
    if (_time < 3.0f)
    {
        auto scoreText = std::to_string(static_cast<int>(_progressManager.getPlayerInfo().points * std::min(_time, 2.0f) * 0.5f));
        _document->getControlByName("score")->setText("Score: " + scoreText);

        if (_stars > 0)
        {
            std::stringstream ss;
            ss << "windows/gameover_" << _countStars << ".png";
            _document->getControlByName("panel")->setDefaultMaterial(ss.str());
            _countStars = std::min(_time * 1.5f, 3.0f) / (3.0 / _stars);
        }
    }
    else if (!_countingFinished)
    {
        _countingFinished = true;
        _document->getControlByName("score")->setText("Score: " + std::to_string(_progressManager.getPlayerInfo().points));

        std::stringstream ss;
        ss << "windows/gameover_" << _stars << ".png";
        _document->getControlByName("panel")->setDefaultMaterial(ss.str());
    }

    return GameState::Score;
}

void ScoreStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void ScoreStateProcessor::show()
{
    auto currentLevel = _progressManager.getCurrentLevel();
    _document->show();
    _document->getControlByName("result")->setText(_progressManager.isVictory() ? "Victory" : "You lose");
    _document->getControlByName("levelname")->setText("Level " + std::to_string(currentLevel));
    _document->getControlByName("time")->setText("Time: " + timeToString(_progressManager.getPlayerInfo().time));
    _document->getControlByName("score")->setText("Score: 0");
    _document->getControlByName("panel")->setDefaultMaterial("windows/gameover_0.png");

    auto restartBtn = _document->getControlByName("restart");
    if (!_progressManager.isVictory())
    {
        int x, y;
        auto continueBtn = _document->getControlByName("continue");
        continueBtn->setVisible(false);
        auto pos = continueBtn->getPosition();
        restartBtn->setPosition(pos);
    } else {
        restartBtn->setPosition({_restartX, restartBtn->getPosition().y});
    }

    auto& scoresManager = Game::getInstance()->getScoresManager();
    auto bestStars = scoresManager.getStars(currentLevel);
    auto bestTime = scoresManager.getTime(currentLevel);

    _stars = 0;
    if (_progressManager.isVictory())
    {
        // TODO: Add stars logic
        _stars = 3;
        if (_progressManager.getPlayerInfo().time > 200)
            --_stars;
        if (_progressManager.getPlayerInfo().time > 250)
            --_stars;

        if (_stars > bestStars)
            scoresManager.setStars(currentLevel, _stars);
        if (bestTime > _progressManager.getPlayerInfo().time)
            scoresManager.setTime(currentLevel, _progressManager.getPlayerInfo().time);
    }

    if (scoresManager.getBestScore() < _progressManager.getPlayerInfo().points)
        scoresManager.setBestScore(_progressManager.getPlayerInfo().points);
    scoresManager.store();

    _time = 0;
    _countStars = 0;
    _countingFinished = false;
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
            auto nextLevel = _progressManager.getCurrentLevel() + 1;
            if (nextLevel > LevelsCount)
                nextLevel = 1;
            _progressManager.setCurrentLevel(nextLevel);
            Game::getInstance()->setState(GameState::Level);
        }

        if (control->getName() == "restart")
        {
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            _progressManager.resetPlayerInfo();
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