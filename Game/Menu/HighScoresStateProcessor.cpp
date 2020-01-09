// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "HighScoresStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <Game/Utils.h>
#include <Game/SystemApi.h>

namespace Chewman
{

HighscoresStateProcessor::HighscoresStateProcessor()
        : _documentPoints(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/highscoresWide.xml" : "resources/game/GUI/highscores.xml"))
        , _documentTime(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/highscoresTimeWide.xml" : "resources/game/GUI/highscoresTime.xml"))
{
    _documentPoints->setMouseUpHandler(this);
    _documentPoints->hide();

    _documentTime->setMouseUpHandler(this);
    _documentTime->hide();

    _documentPoints->getControlByName("refresh")->setDefaultMaterial("refreshing1.png");
    _documentTime->getControlByName("refresh")->setDefaultMaterial("refreshing2.png");
}

HighscoresStateProcessor::~HighscoresStateProcessor() = default;

GameState HighscoresStateProcessor::update(float deltaTime)
{
    auto logged = System::isLoggedServices();
    if (_logged != logged)
    {
        _logged = logged;
        if (_logged)
        {
            // TODO: Submit only if it wasn't submitted already
            System::submitScore(Game::getInstance()->getScoresManager().getBestScore());
        }
        updateBoard();
    }

    if (_logged)
    {
        if (!_isScoresUpdated && System::isScoresUpdated())
        {
            _isScoresUpdated = true;
            updateBoard();
        }

        if (System::hasNewTimeScores())
        {
            updateBoard();
        }

        if (System::hasTimeScoresUpdated())
        {
            if (_isRefreshIcon)
            {
                _documentPoints->getControlByName("refresh")->setDefaultMaterial("buttons/restart.png");
                _documentTime->getControlByName("refresh")->setDefaultMaterial("buttons/restart.png");
                _isRefreshIcon = false;
            }
        } else {
            if (!_isRefreshIcon)
            {
                _isRefreshIcon = true;
                _refreshTime = -1.0f;
            }
        }

        if (_isRefreshIcon)
        {
            _refreshTime -= deltaTime;
            if (_refreshTime < 0)
            {
                _refreshTime = 0.4;
                if (_refreshIconIndex == 1)
                {
                    _refreshIconIndex = 2;
                    _documentPoints->getControlByName("refresh")->setDefaultMaterial("refreshing1.png");
                    _documentTime->getControlByName("refresh")->setDefaultMaterial("refreshing1.png");
                } else {
                    _refreshIconIndex = 1;
                    _documentPoints->getControlByName("refresh")->setDefaultMaterial("refreshing2.png");
                    _documentTime->getControlByName("refresh")->setDefaultMaterial("refreshing2.png");
                }
            }
        }
    } else {
        _documentPoints->getControlByName("refresh")->setVisible(false);
        _documentTime->getControlByName("refresh")->setVisible(false);
    }

    return GameState::Highscores;
}

void HighscoresStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, getCurrentDoc());

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void HighscoresStateProcessor::show()
{
    _documentPoints->show();
    _isPointsActive = true;
    _isWeeklyActive = true;
    _isFirstLevelsHalf = true;
    _isScoresUpdated = false;
    //if (System::isLoggedServices() && !_scoresUpdated)
    //{
    //    System::updateScores();
    //    _scoresUpdated = true;
    //}
    updatePointsDoc();

    System::showAds(System::AdHorizontalLayout::Right, System::AdVerticalLayout::Top);
}

void HighscoresStateProcessor::hide()
{
    _documentPoints->hide();
    _documentTime->hide();
}

bool HighscoresStateProcessor::isOverlapping()
{
    return true;
}

void HighscoresStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            Game::getInstance()->setState(GameState::MainMenu);
        }
        if (control->getName() == "points")
        {
            _documentTime->hide();
            _documentPoints->show();
            _isPointsActive = true;
            updatePointsDoc();
        }
        if (control->getName() == "time")
        {
            _documentTime->show();
            _documentPoints->hide();
            _isPointsActive = false;
            updateTimeDoc();
        }
        if (control->getName() == "weekly" || control->getName() == "weeklyLabel")
        {
            _isWeeklyActive = true;
            updateCheckboxes();
            updateBoard();
        }
        if (control->getName() == "alltime" || control->getName() == "alltimeLabel")
        {
            _isWeeklyActive = false;
            updateCheckboxes();
            updateBoard();
        }
        if (control->getName() == "left" && !_isFirstLevelsHalf)
        {
            _isFirstLevelsHalf = true;
            updateTimeScores();

        }
        if (control->getName() == "right" && _isFirstLevelsHalf)
        {
            _isFirstLevelsHalf = false;
            updateTimeScores();
        }
        if (control->getName() == "refresh")
        {
            if (System::isLoggedServices() && System::hasTimeScoresUpdated())
                System::refreshScores();
        }
    }
}

ControlDocument* HighscoresStateProcessor::getCurrentDoc()
{
    return _isPointsActive ? _documentPoints.get() : _documentTime.get();
}

void HighscoresStateProcessor::updateBoard()
{
    if (_isPointsActive)
        updatePointsDoc();
    else
        updateTimeScores();
}

void HighscoresStateProcessor::updatePointsDoc()
{
    updateCheckboxes();
    updatePointScores();
}

void HighscoresStateProcessor::updateTimeDoc()
{
    updateCheckboxes();
    updateTimeScores();
}

void HighscoresStateProcessor::updateCheckboxes()
{
    auto* currentDoc = getCurrentDoc();
    currentDoc->getControlByName("weekly")->setDefaultMaterial(_isWeeklyActive ? "buttons/checkbox_checked.png" : "buttons/checkbox_unchecked.png");
    currentDoc->getControlByName("alltime")->setDefaultMaterial(_isWeeklyActive ? "buttons/checkbox_unchecked.png" : "buttons/checkbox_checked.png");
}

void HighscoresStateProcessor::updateTimeScores()
{
    int shift = _isFirstLevelsHalf ? 0 : 18;
    auto timeData = System::getTimes(_isWeeklyActive);

    auto getLevelStr = [](int num)
    {
        auto levelNumStr = std::to_string(num);
        if (levelNumStr.size() == 1)
            levelNumStr.insert(levelNumStr.begin(), '0');
        return levelNumStr;
    };

    for (auto i = 0; i < 18; i++)
    {
        auto controlName = "lvl" + getLevelStr(i + 1);
        glm::vec4 textColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        auto& scoresManager = Game::getInstance()->getScoresManager();
        std::string time = "-:--";
        std::string playerName = System::getPlayerName();
        uint32_t levelNum = i + 1 + shift;

        if (System::isLoggedServices() && timeData.size() >= levelNum)
        {
            if (timeData[levelNum - 1].second != 0)
            {
                time = Utils::timeToString(timeData[levelNum - 1].second / 1000);
                playerName = timeData[levelNum - 1].first;
            } else if (scoresManager.getTime(i + 1 + shift) != 0)
            {
                time = Utils::timeToString(scoresManager.getTime(i + 1 + shift));
                textColor = { 0.6f, 0.6f, 0.8f, 1.0f };
            } else {
                textColor = { 0.6f, 0.6f, 0.8f, 1.0f };
            }
        } else
        {
            textColor = { 0.8f, 0.8f, 0.9f, 1.0f };
            if (scoresManager.getTime(i + 1 + shift) != 0)
            {
                time = Utils::timeToString(scoresManager.getTime(i + 1 + shift));
            }
        }

        std::string level = "L" + getLevelStr(levelNum);
        _documentTime->getControlByName(controlName)->setText(level.append(": ").append(playerName).append(" / ").append(time));
        _documentTime->getControlByName(controlName)->setTextColor(textColor);
    }

    _documentTime->getControlByName("left")->setVisible(!_isFirstLevelsHalf);
    _documentTime->getControlByName("right")->setVisible(_isFirstLevelsHalf);
}

void HighscoresStateProcessor::updatePointScores()
{
    if (System::isLoggedServices())
    {
        auto scoreData = System::getScores(_isWeeklyActive);

        for (auto i = 1; i <= 5; i++)
        {
            auto indexStr = std::to_string(i);
            if (i <= scoreData.size() && scoreData[i - 1].second != 0)
            {
                _documentPoints->getControlByName("name" + indexStr)->setText(
                        indexStr + ". " + scoreData[i - 1].first);
                _documentPoints->getControlByName("score" + indexStr)->setText(std::to_string(scoreData[i - 1].second));
            } else {
                _documentPoints->getControlByName("name" + indexStr)->setText(" ");
                _documentPoints->getControlByName("score" + indexStr)->setText(" ");
            }
        }

        auto playerPlace = System::getPlayerPlace(_isWeeklyActive);
        if (playerPlace <= 5)
        {
            _documentPoints->getControlByName("name6")->setText(" ");
            _documentPoints->getControlByName("score6")->setText(" ");
        } else {
            auto playerScore = System::getPlayerScore(_isWeeklyActive);
            _documentPoints->getControlByName("name6")->setText(std::to_string(playerPlace) + ". " + playerScore.first);
            _documentPoints->getControlByName("score6")->setText(std::to_string(playerScore.second));
        }
    }
    else
    {
        for (auto i = 1; i <= 6; i++)
        {
            // TODO: Implement high scores manager for several values
            auto indexStr = std::to_string(i);
            _documentPoints->getControlByName("name" + indexStr)->setText(" ");
            _documentPoints->getControlByName("score" + indexStr)->setText(" ");
        }

        _documentPoints->getControlByName("name1")->setText("1. Player");
        _documentPoints->getControlByName("score1")->setText(
                std::to_string(Game::getInstance()->getScoresManager().getBestScore()));
    }
}

} // namespace Chewman