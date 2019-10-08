// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "MenuStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

MenuStateProcessor::MenuStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/startmenu.xml");
    _document->setMouseUpHandler(this);
    _document->hide();
}

MenuStateProcessor::~MenuStateProcessor() = default;

GameState MenuStateProcessor::update(float deltaTime)
{
    return GameState::MainMenu;
}

void MenuStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void MenuStateProcessor::show()
{
    _document->show();
}

void MenuStateProcessor::hide()
{
    _document->hide();
}

bool MenuStateProcessor::isOverlapping()
{
    return false;
}

void MenuStateProcessor::ProcessEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "start")
        {
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(11);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            progressManager.resetPlayerInfo();
            Game::getInstance()->setState(GameState::Level);
        }
    }
}

} // namespace Chewman