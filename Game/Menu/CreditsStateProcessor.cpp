// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "CreditsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

CreditsStateProcessor::CreditsStateProcessor()
        : _document(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/creditsWide.xml" : "resources/game/GUI/credits.xml"))
{
    _document->setMouseUpHandler(this);
    _document->hide();
}

CreditsStateProcessor::~CreditsStateProcessor() = default;

GameState CreditsStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::Credits;
}

void CreditsStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void CreditsStateProcessor::show()
{
    _document->show();
}

void CreditsStateProcessor::hide()
{
    _document->hide();
}

bool CreditsStateProcessor::isOverlapping()
{
    return true;
}

void CreditsStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
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