// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ScoreStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

ScoreStateProcessor::ScoreStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/scoremenu.xml");
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

ScoreStateProcessor::~ScoreStateProcessor() = default;

GameState ScoreStateProcessor::update(float deltaTime)
{
    return GameState::Score;
}

void ScoreStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void ScoreStateProcessor::show()
{
    _document->show();
}

void ScoreStateProcessor::hide()
{
    _document->hide();
}

bool ScoreStateProcessor::isOverlapping()
{
    return true;
}

void ScoreStateProcessor::ProcessEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "continue")
        {
            Game::getInstance()->setState(GameState::Level);
        }

        if (control->getName() == "restart")
        {
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            Game::getInstance()->setState(GameState::Level);
        }

        if (control->getName() == "tomenu")
        {
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(1);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            Game::getInstance()->setState(GameState::MainMenu);
        }
    }
}

} // namespace Chewman