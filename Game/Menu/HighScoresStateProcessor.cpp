// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "HighScoresStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <Game/Utils.h>

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
}

HighscoresStateProcessor::~HighscoresStateProcessor() = default;

GameState HighscoresStateProcessor::update(float deltaTime)
{
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
    updatePointsDoc();
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
        }
        if (control->getName() == "alltime" || control->getName() == "alltimeLabel")
        {
            _isWeeklyActive = false;
            updateCheckboxes();
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
    }
}

ControlDocument* HighscoresStateProcessor::getCurrentDoc()
{
    return _isPointsActive ? _documentPoints.get() : _documentTime.get();
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

        auto& scoresManager= Game::getInstance()->getScoresManager();
        std::string time = "-:--";
        if (scoresManager.getTime(i + 1 + shift) != 0)
        {
            time = Utils::timeToString(scoresManager.getTime(i + 1 + shift));
        }

        _documentTime->getControlByName(controlName)->setText("L" + getLevelStr(i + 1 + shift) + ": Player / " + time);
    }

    _documentTime->getControlByName("left")->setVisible(!_isFirstLevelsHalf);
    _documentTime->getControlByName("right")->setVisible(_isFirstLevelsHalf);
}

void HighscoresStateProcessor::updatePointScores()
{
    for (auto i = 1; i <= 5; i++)
    {
        // TODO: Implement high scores manager for several values
        auto indexStr = std::to_string(i);
        _documentPoints->getControlByName("name" + indexStr)->setText(" ");
        _documentPoints->getControlByName("score" + indexStr)->setText(" ");
    }

    _documentPoints->getControlByName("name1")->setText("1. Player");
    _documentPoints->getControlByName("score1")->setText(std::to_string(Game::getInstance()->getScoresManager().getBestScore()));
}

} // namespace Chewman