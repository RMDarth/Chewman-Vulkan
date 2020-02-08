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

    const auto& language = Game::getInstance()->getLocaleManager().getLanguage();
    if (language != "fr" && language != "de")
    {
        const auto shiftControls = [](ControlDocument* document)
        {
            auto allTimeControl = document->getControlByName("alltime");
            auto allTimeSize = allTimeControl->getSize();
            auto allTimePos = allTimeControl->getPosition();
            allTimeControl->setPosition({allTimePos.x - allTimeSize.x, allTimePos.y});

            auto allTimeLabelControl = document->getControlByName("alltimeLabel");
            auto allTimeLabelPos = allTimeLabelControl->getPosition();
            allTimeLabelControl->setPosition({ allTimeLabelPos.x - allTimeSize.x, allTimeLabelPos.y});
        };

        shiftControls(_documentPoints.get());
        shiftControls(_documentTime.get());
    }

    _isRefreshIcon = true;
}

HighscoresStateProcessor::~HighscoresStateProcessor() = default;

GameState HighscoresStateProcessor::update(float deltaTime)
{
    if (_logged && _isRefreshIcon)
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

    _updateTime -= deltaTime;
    if (_updateTime > 0)
        return GameState::Highscores;

    _updateTime = 2.0f;

    auto logged = System::isLoggedServices();
    if (_logged != logged)
    {
        _logged = logged;
        if (_logged)
        {
            if (Game::getInstance()->getScoresManager().isNewBestScore())
            {
                System::submitScore(Game::getInstance()->getScoresManager().getBestScore());
                Game::getInstance()->getScoresManager().setIsNewScore(false);
            }
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
    if (!System::isLoggedServices())
    {
        if (System::isSignedServices() && !_signInTried)
        {
            _signInTried = true;
            System::logInServices();
        }
        _documentPoints->getControlByName("refresh")->setVisible(false);
    }

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
        if (control->getName() == "googleplay")
        {
            if (System::isLoggedServices())
                System::showLeaderboard(_isWeeklyActive);
            else
                System::logInServices();
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
    if (!_logged)
        currentDoc->getControlByName("refresh")->setVisible(false);
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
            auto playerScore = scoresManager.getTime(i + 1 + shift);
            if (timeData[levelNum - 1].second != 0 )
            {
                if (!_isWeeklyActive && playerScore != 0 && playerScore < (timeData[levelNum - 1].second / 1000))
                {
                    time = Utils::timeToString(playerScore);
                } else
                {
                    time = Utils::timeToString(timeData[levelNum - 1].second / 1000);
                    playerName = timeData[levelNum - 1].first;
                }
            } else if (playerScore != 0)
            {
                time = Utils::timeToString(playerScore);
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
        bool playerInserted = false;
        auto bestScore = Game::getInstance()->getScoresManager().getBestScore();

        if (bestScore != 0 && !_isWeeklyActive &&
            (scoreData.empty() || scoreData.size() < 5 || bestScore > scoreData.back().second))
        {
            std::string playerName = System::getPlayerName();

            auto playerPos = std::find_if(scoreData.begin(), scoreData.end(), [&](auto item) { return item.first == playerName; });
            if (playerPos != scoreData.end())
            {
                if (playerPos->second < bestScore)
                    (*playerPos).second = bestScore;
            } else {
                scoreData.emplace_back(playerName, bestScore);
            }
            std::sort(scoreData.begin(), scoreData.end(), [](auto a, auto b) { return a.second > b.second; });
            playerInserted = true;
        }

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

        if (!playerInserted)
        {
            auto playerPlace = System::getPlayerPlace(_isWeeklyActive);
            if (playerPlace <= 5)
            {
                _documentPoints->getControlByName("name6")->setText(" ");
                _documentPoints->getControlByName("score6")->setText(" ");
            } else
            {
                auto playerScore = System::getPlayerScore(_isWeeklyActive);
                if (!_isWeeklyActive && playerScore.second < bestScore)
                    playerScore.second = bestScore;
                _documentPoints->getControlByName("name6")->setText(
                        std::to_string(playerPlace) + ". " + playerScore.first);
                _documentPoints->getControlByName("score6")->setText(std::to_string(playerScore.second));
            }
        } else {
            _documentPoints->getControlByName("name6")->setText(" ");
            _documentPoints->getControlByName("score6")->setText(" ");
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