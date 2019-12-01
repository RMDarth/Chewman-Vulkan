// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "TutorialStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

TutorialStateProcessor::TutorialStateProcessor()
    : _document(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/tutorialWide.xml" : "resources/game/GUI/tutorial.xml"))
{

    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

TutorialStateProcessor::~TutorialStateProcessor() = default;

GameState TutorialStateProcessor::update(float deltaTime)
{
    return GameState::Tutorial;
}

void TutorialStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void TutorialStateProcessor::show()
{
    auto tutorialData = Game::getInstance()->getTutorialData();
    for (auto i = 0; i < tutorialData.size(); ++i)
    {
        _document->getControlByName("text" + std::to_string(i + 1))->setText(tutorialData[i]);
    }
    _document->show();
}

void TutorialStateProcessor::hide()
{
    _document->hide();
}

bool TutorialStateProcessor::isOverlapping()
{
    return true;
}

void TutorialStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "continue")
        {
            Game::getInstance()->setState(GameState::Level);
        }
    }
}

} // namespace Chewman