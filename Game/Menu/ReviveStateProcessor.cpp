// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ReviveStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>
#include <Game/SystemApi.h>

namespace Chewman
{

ReviveStateProcessor::ReviveStateProcessor()
        : _document(std::make_unique<ControlDocument>("resources/game/GUI/revive.xml"))
{
    _document->setMouseUpHandler(this);
    _document->hide();
}

ReviveStateProcessor::~ReviveStateProcessor() = default;

GameState ReviveStateProcessor::update(float deltaTime)
{
    if (_videoAdsLaunched)
    {
        if (System::wasVideoAdsWatched())
        {
            Game::getInstance()->getSoundsManager().unpauseMusic();
            _videoAdsLaunched = false;
            _document->hide();
            return GameState::Pause;
        }
    }

    return GameState::Revive;
}

void ReviveStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::Score);
    }
}

void ReviveStateProcessor::show()
{
    _document->show();
    _videoAdsLaunched = false;
    System::showAds(System::AdHorizontalLayout::Center, System::AdVerticalLayout::Bottom);
}

void ReviveStateProcessor::hide()
{
    Game::getInstance()->getSoundsManager().unpauseMusic();
    _document->hide();
}

bool ReviveStateProcessor::isOverlapping()
{
    return true;
}

void ReviveStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "revive")
        {
            if (System::showVideoAds())
            {
                _videoAdsLaunched = true;
                Game::getInstance()->getSoundsManager().pauseMusic();
            }
        }

        if (control->getName() == "skip")
        {
            _document->hide();
            Game::getInstance()->getSoundsManager().unpauseMusic();
            Game::getInstance()->getProgressManager().setStarted(false);
            Game::getInstance()->setState(GameState::Score);
        }
    }
}

} // namespace Chewman