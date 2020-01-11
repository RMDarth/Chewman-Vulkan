// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "WorldSelectionStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Controls/BoxSliderControl.h"
#include "Game/SystemApi.h"
#include "Game/Game.h"

namespace Chewman
{

constexpr uint16_t unlockStarCount = 70;

WorldSelectionStateProcessor::WorldSelectionStateProcessor()
    : _document(std::make_unique<ControlDocument>("resources/game/GUI/worldselectmenu.xml"))
{
    _document->setMouseUpHandler(this);
    _document->hide();

    _slider = static_cast<BoxSliderControl*>(_document->getControlByName("slider").get());
    _lockControl = _document->getControlByName("star").get();
    _isLockedLevels = !System::isItemBought(System::levelsProduct);
    if (Game::getInstance()->getScoresManager().getTotalStars() >= unlockStarCount)
        _isLockedLevels = false;
}

WorldSelectionStateProcessor::~WorldSelectionStateProcessor() = default;

GameState WorldSelectionStateProcessor::update(float deltaTime)
{
    static std::string campaignNames[] = {
            "Exordium",
            "Deeper dungeons",
            "Hell's gate"
    };
    _document->update(deltaTime);

    if (_slider->getSelectedObject() != _currentWorld)
    {
        _currentWorld = _slider->getSelectedObject();
        _document->getControlByName("campaignName")->setText(campaignNames[_currentWorld]);
    }
    if (_currentWorld == 2 && !_slider->isSliding())
    {
        if (!_lockControl->isVisible() && _isLockedLevels)
            setLockControlsVisible(true);
    } else {
        setLockControlsVisible(false);
    }

    if (_currentWorld == 2 && _isLockedLevels)
    {
        _isLockedLevels = !System::isItemBought(System::levelsProduct);
        if (!_isLockedLevels && _lockControl->isVisible())
        {
            _document->getControlByName("world3")->setDefaultMaterial("worlds/world3.jpg");
            setLockControlsVisible(false);
        }
    }

    return GameState::WorldSelection;
}

void WorldSelectionStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void WorldSelectionStateProcessor::show()
{
    _document->show();
    if (Game::getInstance()->getScoresManager().getTotalStars() >= unlockStarCount)
        _isLockedLevels = false;

    setLockControlsVisible(_currentWorld == 2 && _isLockedLevels);
    _document->getControlByName("world3")->setDefaultMaterial(_isLockedLevels ? "worlds/world3lock.jpg" : "worlds/world3.jpg");

    System::showAds(System::AdHorizontalLayout::Center, System::AdVerticalLayout::Bottom);
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
            if (_isLockedLevels && _currentWorld == 2)
            {
                System::buyItem(System::levelsProduct);
            }
            else
            {
                hide();
                auto worldNum = static_cast<BoxSliderControl*>(control)->getSelectedObject();
                Game::getInstance()->getProgressManager().setCurrentWorld(worldNum);
                Game::getInstance()->setState(GameState::LevelSelection);
            }
        }
    }
}

void WorldSelectionStateProcessor::setLockControlsVisible(bool visible)
{
    if (_lockControl->isVisible() != visible)
    {
        _lockControl->setVisible(visible);
        _document->getControlByName("unlockstars")->setVisible(visible);
        _document->getControlByName("unlockmoney")->setVisible(visible);

        if (visible)
        {
            std::string text = std::to_string(Game::getInstance()->getScoresManager().getTotalStars()) + "/" + std::to_string(unlockStarCount);
            _document->getControlByName("unlockstars")->setText(text);
        }
    }
}

} // namespace Chewman