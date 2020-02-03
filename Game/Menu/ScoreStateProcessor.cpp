// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ScoreStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Game.h"
#include "Game/Utils.h"
#include "Game/SystemApi.h"
#include <sstream>

namespace Chewman
{

ScoreStateProcessor::ScoreStateProcessor()
    : _document(std::make_unique<ControlDocument>("resources/game/GUI/scoremenu.xml"))
    , _progressManager(Game::getInstance()->getProgressManager())

{
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();

    _restartX = _document->getControlByName("restart")->getPosition().x;
    for (auto i = 0; i < 3; ++i)
    {
        _document->getControlByName("starSpark" + std::to_string(i+1))->setRawMaterial("OverlaySparksMaterial");
    }

    _scoreLabel = Game::getInstance()->getLocaleManager().getLocalizedString("ScorePointLabel");
    _timeLabel = Game::getInstance()->getLocaleManager().getLocalizedString("ScoreTimeLabel");
}

ScoreStateProcessor::~ScoreStateProcessor() = default;

GameState ScoreStateProcessor::update(float deltaTime)
{
    _time += deltaTime;
    if (_time < 3.0f)
    {
        auto scoreText = std::to_string(static_cast<int>(_progressManager.getPlayerInfo().points * std::min(_time, 2.0f) * 0.5f));
        _document->getControlByName("score")->setText(_scoreLabel + ": " + scoreText);

        auto prevCountStars = _countStars;
        if (_stars > 0)
        {
            std::stringstream ss;
            ss << "windows/gameover_" << _countStars << ".png";
            _document->getControlByName("panel")->setDefaultMaterial(ss.str());
            _countStars = std::min(_time * 1.5f, 3.0f) / (3.0 / _stars);
        }
        if (_countStars > prevCountStars)
        {
            Game::getInstance()->getSoundsManager().playSound(SoundType::Star1);
        }
    }
    else if (!_countingFinished)
    {
        _countingFinished = true;
        _document->getControlByName("score")->setText(_scoreLabel + ": " + std::to_string(_progressManager.getPlayerInfo().points));

        std::stringstream ss;
        ss << "windows/gameover_" << _stars << ".png";
        _document->getControlByName("panel")->setDefaultMaterial(ss.str());
    }

    if (_time < 4.0f)
    {
        if (_stars > 0 && _countStars > 0)
        {
            for (auto i = 0; i < _countStars; ++i)
            {
                _startStars[i] += deltaTime;
                if (_startStars[i] > 1.0f) _startStars[i] = 1.0f;
                _document->getControlByName("starSpark" + std::to_string(i + 1))->getOverlay()->setCustomData(
                        glm::vec4(5, 5, _startStars[i], 0));
            }
        }
    }

    return GameState::Score;
}

void ScoreStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        _progressManager.setVictory(false);
        _progressManager.setStarted(false);
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void ScoreStateProcessor::show()
{
    auto currentLevel = _progressManager.getCurrentLevel();
    _document->show();
    _document->getControlByName("result")->setText(
         Game::getInstance()->getLocaleManager().getLocalizedString(_progressManager.isVictory() ? "ScoreVictory" : "ScoreGameOver"));
    _document->getControlByName("levelname")->setText(
         Game::getInstance()->getLocaleManager().getLocalizedString("LevelShort") + std::to_string(currentLevel) + ": " + _progressManager.getCurrentLevelInfo().levelName);
    _document->getControlByName("time")->setText(_timeLabel + ": " + Utils::timeToString(_progressManager.getPlayerInfo().time));
    _document->getControlByName("score")->setText(_scoreLabel + ": 0");
    _document->getControlByName("panel")->setDefaultMaterial("windows/gameover_0.png");

    _document->getControlByName("highscoreScore")->setVisible(false);
    _document->getControlByName("highscoreTime")->setVisible(false);

    _document->getControlByName("scoreTime2")->setText(Utils::timeToString(_progressManager.getCurrentLevelInfo().timeFor2Stars));
    _document->getControlByName("scoreTime3")->setText(Utils::timeToString(_progressManager.getCurrentLevelInfo().timeFor3Stars));


    for (auto i = 0; i < 3; ++i)
    {
        _startStars[i] = 0;
        _document->getControlByName("starSpark" + std::to_string(i+1))->getOverlay()->setCustomData(glm::vec4(5, 5, 0, 0));
    }

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
        _stars = 3;
        if (_progressManager.getPlayerInfo().livesLostOnLevel == 0)
        {
            _stars = 3;
        }
        else
        {
            if (_progressManager.getPlayerInfo().time > _progressManager.getCurrentLevelInfo().timeFor3Stars)
                --_stars;
            if (_progressManager.getPlayerInfo().time > _progressManager.getCurrentLevelInfo().timeFor2Stars)
                --_stars;
        }

        if (_stars > bestStars)
            scoresManager.setStars(currentLevel, _stars);
        if (bestTime == 0 || bestTime > _progressManager.getPlayerInfo().time)
        {
            scoresManager.setTime(currentLevel, _progressManager.getPlayerInfo().time);
            bestTime = _progressManager.getPlayerInfo().time;

            _document->getControlByName("highscoreTime")->setVisible(true);
        }
    }

    if (scoresManager.getBestScore() < _progressManager.getPlayerInfo().points)
    {
        scoresManager.setBestScore(_progressManager.getPlayerInfo().points);
        _document->getControlByName("highscoreScore")->setVisible(true);
    }

    if (System::isLoggedServices())
    {
        if (bestTime > 0 && scoresManager.isNewTime(currentLevel))
        {
            System::submitLevelTime(currentLevel - 1, bestTime);
            scoresManager.setIsNewTimeScore(currentLevel, false);
        } else if (_progressManager.isVictory())
        {
            System::submitLevelTime(currentLevel - 1, _progressManager.getPlayerInfo().time, false);
        }

        System::submitScore(_progressManager.getPlayerInfo().points, false);

        if (scoresManager.getStars(1) > 0 && scoresManager.getStars(2) > 0)
            System::unlockAchievement(System::AchievementType::Newcomer);
        uint32_t totalLevels = 0;
        uint32_t total3StarLevels = 0;
        uint32_t totalStars = scoresManager.getTotalStars();
        for (auto i = 1; i <= 36; i++)
        {
            if (scoresManager.getStars(i) > 0) ++totalLevels;
            if (scoresManager.getStars(i) == 3) ++total3StarLevels;
        }

        System::updateAchievement(System::AchievementType::Good_start, totalLevels);
        System::updateAchievement(System::AchievementType::Treasure_hunter, totalLevels);
        System::updateAchievement(System::AchievementType::Dungeon_master, totalLevels);
        System::updateAchievement(System::AchievementType::Dungeon_emperor, totalLevels);

        System::updateAchievement(System::AchievementType::Star_collector, totalStars);
        System::updateAchievement(System::AchievementType::Star_hunter, totalStars);
        System::updateAchievement(System::AchievementType::Supernova, totalStars);

        System::updateAchievement(System::AchievementType::Gold_rush, total3StarLevels);
        System::updateAchievement(System::AchievementType::Mission_accomplished, total3StarLevels);
    } else {
        _document->getControlByName("googleplay")->setVisible(false);
    }

    scoresManager.store();

    _time = 0;
    _countStars = 0;
    _countingFinished = false;

    System::showAds(System::AdHorizontalLayout::Right, System::AdVerticalLayout::Top);
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

        if (control->getName() == "googleplay")
        {
            System::showTimeLeaderboard(_progressManager.getCurrentLevel() - 1, false);
        }
    }
}

} // namespace Chewman