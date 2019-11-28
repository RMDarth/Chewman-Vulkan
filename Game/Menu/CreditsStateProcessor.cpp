// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "CreditsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

CreditsStateProcessor::CreditsStateProcessor()
        : _document(std::make_unique<ControlDocument>("resources/game/GUI/credits.xml"))
{
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

CreditsStateProcessor::~CreditsStateProcessor() = default;

GameState CreditsStateProcessor::update(float deltaTime)
{
    return GameState::Credits;
}

void CreditsStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void CreditsStateProcessor::show()
{
    auto tutorialData = Game::getInstance()->getTutorialData();
    for (auto i = 0; i < tutorialData.size(); ++i)
    {
        _document->getControlByName("text" + std::to_string(i + 1))->setText(tutorialData[i]);
    }
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