// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "PauseStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <Game/SystemApi.h>

namespace Chewman
{

PauseStateProcessor::PauseStateProcessor()
    : _document(std::make_unique<ControlDocument>("resources/game/GUI/pausemenu.xml"))
{
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
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::Level);
    }
}

void PauseStateProcessor::show()
{
    _document->show();
    System::showAds(System::AdHorizontalLayout::Center, System::AdVerticalLayout::Bottom);
    resetControls(false);
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

        if (control->getName() == "reset")
        {
            auto& progressManager = Game::getInstance()->getProgressManager();
            if (progressManager.getPlayerInfo().lives > 0)
            {
                progressManager.setVictory(false);
                progressManager.setStarted(false);
                progressManager.getPlayerInfo().lives--;
                Game::getInstance()->setState(GameState::Level);
            }
        }

        if (control->getName() == "daynight")
        {
            if (auto* gameService = Game::getInstance()->getProgressManager().getGameMapService())
            {
                gameService->switchDayNight();
                Game::getInstance()->setState(GameState::Level);
            }
        }

        if (control->getName() == "tomenu")
        {
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(1);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            Game::getInstance()->setState(GameState::MainMenu);
        }

        if (control->getName() == "more")
        {
            resetControls(!_isCurrentControlsExtended);
        }
    }
}

void PauseStateProcessor::resetControls(bool isExtended)
{
    _document->getControlByName("reset")->setVisible(isExtended);
    _document->getControlByName("daynight")->setVisible(isExtended && Game::getInstance()->getGraphicsManager().getSettings().dynamicLights != LightSettings::Off);

    if (isExtended)
    {
        if (Game::getInstance()->getProgressManager().getPlayerInfo().lives == 0)
            _document->getControlByName("reset")->setDefaultMaterial("buttons/button_disabled.png");
        else
            _document->getControlByName("reset")->setDefaultMaterial("buttons/button_normal.png");
    }

    _document->getControlByName("continue")->setVisible(!isExtended);
    _document->getControlByName("restart")->setVisible(!isExtended);
    _document->getControlByName("tomenu")->setVisible(!isExtended);
    _document->getControlByName("more")->setDefaultMaterial(isExtended ? "buttons/up.png" : "buttons/down.png");
    _document->getControlByName("more")->setHoverMaterial(isExtended ? "buttons/up.png" : "buttons/down.png");

    _isCurrentControlsExtended = isExtended;
}

} // namespace Chewman