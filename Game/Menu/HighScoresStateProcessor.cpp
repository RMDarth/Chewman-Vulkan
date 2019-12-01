// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "HighScoresStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

HighscoresStateProcessor::HighscoresStateProcessor()
        : _document(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/highscoresWide.xml" : "resources/game/GUI/highscores.xml"))
{
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

HighscoresStateProcessor::~HighscoresStateProcessor() = default;

GameState HighscoresStateProcessor::update(float deltaTime)
{
    return GameState::Highscores;
}

void HighscoresStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void HighscoresStateProcessor::show()
{
    for (auto i = 1; i <= 5; i++)
    {
        // TODO: Implement high scores manager for several values
        auto indexStr = std::to_string(i);
        _document->getControlByName("name" + indexStr)->setText(" ");
        _document->getControlByName("score" + indexStr)->setText(" ");
    }

    _document->getControlByName("name1")->setText("1. Player");
    _document->getControlByName("score1")->setText(std::to_string(Game::getInstance()->getScoresManager().getBestScore()));

    _document->show();
}

void HighscoresStateProcessor::hide()
{
    _document->hide();
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
    }
}

} // namespace Chewman