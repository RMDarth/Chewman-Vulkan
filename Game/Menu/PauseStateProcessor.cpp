// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "PauseStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

PauseStateProcessor::PauseStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/pausemenu.xml");
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

PauseStateProcessor::~PauseStateProcessor() = default;

GameState PauseStateProcessor::update(float deltaTime)
{
    return GameState::Pause;
}

void PauseStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void PauseStateProcessor::show()
{
    _document->show();
}

void PauseStateProcessor::hide()
{
    _document->hide();
}

bool PauseStateProcessor::isOverlapping()
{
    return true;
}

void PauseStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
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
            progressManager.resetPlayerInfo();
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