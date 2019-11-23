// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "WorldSelectionStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Controls/SliderControl.h"
#include "Game/Game.h"

namespace Chewman
{

WorldSelectionStateProcessor::WorldSelectionStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/worldselectmenu.xml");
    _document->setMouseUpHandler(this);
    _document->hide();
}

WorldSelectionStateProcessor::~WorldSelectionStateProcessor() = default;

GameState WorldSelectionStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::WorldSelection;
}

void WorldSelectionStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void WorldSelectionStateProcessor::show()
{
    _document->show();
}

void WorldSelectionStateProcessor::hide()
{
    _document->hide();
}

bool WorldSelectionStateProcessor::isOverlapping()
{
    return true;
}

void WorldSelectionStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            Game::getInstance()->setState(GameState::MainMenu);
        }
        else if (control->getName() == "slider")
        {
            hide();
            auto worldNum = static_cast<SliderControl*>(control)->getSelectedObject();
            Game::getInstance()->getProgressManager().setCurrentWorld(worldNum);
            Game::getInstance()->setState(GameState::LevelSelection);
        }
    }
}

} // namespace Chewman