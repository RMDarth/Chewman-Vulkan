// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LevelSelectionStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Controls/SliderControl.h"
#include "Game/Game.h"

namespace Chewman
{

LevelSelectionStateProcessor::LevelSelectionStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/levelselectmenu.xml");
    _document->setMouseUpHandler(this);
    _document->hide();
}

LevelSelectionStateProcessor::~LevelSelectionStateProcessor() = default;

GameState LevelSelectionStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::LevelSelection;
}

void LevelSelectionStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void LevelSelectionStateProcessor::show()
{
    _document->show();
}

void LevelSelectionStateProcessor::hide()
{
    _document->hide();
}

bool LevelSelectionStateProcessor::isOverlapping()
{
    return false;
}

void LevelSelectionStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            Game::getInstance()->setState(GameState::WorldSelection);
        }
        else if (control->getType() == ControlType::LevelButton)
        {
            auto levelNum = std::stoul(control->getName());
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(levelNum);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            progressManager.resetPlayerInfo();
            Game::getInstance()->setState(GameState::Level);
        }
    }
}

} // namespace Chewman