// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SettingsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/SystemApi.h"

#include <Game/Game.h>

namespace Chewman
{

const GraphicsSettings _highSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::High, true, LightSettings::High, ParticlesSettings::Partial, EffectSettings::High};
const GraphicsSettings _medSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::Low, true, LightSettings::Simple, ParticlesSettings::None, EffectSettings::Medium};
const GraphicsSettings _lowSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::Low, true, LightSettings::Off, ParticlesSettings::None, EffectSettings::Low};

SettingsStateProcessor::SettingsStateProcessor()
        : _document(std::make_unique<ControlDocument>("resources/game/GUI/settings.xml"))
{
    _document->setMouseUpHandler(this);
    _document->getControlByName("soundSlider")->setMouseMoveHandler(this);
    _document->getControlByName("musicSlider")->setMouseMoveHandler(this);
    _document->getControlByName("soundSlider")->setMouseDownHandler(this);
    _document->getControlByName("musicSlider")->setMouseDownHandler(this);
    _document->hide();
}

SettingsStateProcessor::~SettingsStateProcessor() = default;

GameState SettingsStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);

    if (_playSound)
    {
        if (_soundDelay >= 0)
        {
            _soundDelay -= deltaTime;
        } else {
            Game::getInstance()->getSoundsManager().playSound(SoundType::ChewCoin);
            _soundDelay = 0.3f;
        }
    }

    return GameState::Settings;
}

void SettingsStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void SettingsStateProcessor::show()
{
    _document->show();

    auto& soundManager = Game::getInstance()->getSoundsManager();
    _document->getControlByName("soundSlider")->setCustomAttribute("progress", std::to_string(soundManager.getSoundVolume()));
    _document->getControlByName("musicSlider")->setCustomAttribute("progress", std::to_string(soundManager.getMusicVolume()));

    bool needRestart = Game::getInstance()->getGraphicsManager().needRestart();
    _document->getControlByName("restartInfo")->setVisible(needRestart);
    _document->getControlByName("restart")->setVisible(needRestart);

    setGraphicsSettingsValue();

    _document->getControlByName("gamepadCheckbox")->setDefaultMaterial(
            Game::getInstance()->getGameSettingsManager().getSettings().showOnScreenControls
                 ? "buttons/checkbox_checked.png"
                 : "buttons/checkbox_unchecked.png");

    System::showAds(System::AdHorizontalLayout::Right, System::AdVerticalLayout::Bottom);
}

void SettingsStateProcessor::hide()
{
    _document->hide();
    _document->getControlByName("restartInfo")->setVisible(false);
    _document->getControlByName("restart")->setVisible(false);
    Game::getInstance()->getSoundsManager().save();
}

bool SettingsStateProcessor::isOverlapping()
{
    return true;
}

void SettingsStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            Game::getInstance()->setState(GameState::MainMenu);
        }
        if (control->getName() == "advancedGraphics")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Graphics);
        }

        if (control->getName() == "graphics")
        {
            if (control->getCustomAttribute("listShown") == "true")
                _document->getControlByName("advancedGraphics")->setMouseTransparent(true);
        } else {
            _document->getControlByName("advancedGraphics")->setMouseTransparent(false);
        }

        auto& graphicsManager = Game::getInstance()->getGraphicsManager();

        if (control->getName() == "graphicsHigh")
        {
            graphicsManager.setSettings(_highSettings);
        }
        else if (control->getName() == "graphicsMed")
        {
            graphicsManager.setSettings(_medSettings);
        }
        else if (control->getName() == "graphicsLow")
        {
            graphicsManager.setSettings(_lowSettings);
        }
        else if (control->getName() == "restart")
        {
            System::restartApp();
        }
        else if (control->getName() == "soundSlider")
        {
            _playSound = false;
        }
        else if (control->getName() == "gamepadCheckbox")
        {
            auto& gameSettingsManager = Game::getInstance()->getGameSettingsManager();
            gameSettingsManager.getSettings().showOnScreenControls = !gameSettingsManager.getSettings().showOnScreenControls;
            control->setDefaultMaterial(gameSettingsManager.getSettings().showOnScreenControls
                                            ? "buttons/checkbox_checked.png"
                                            : "buttons/checkbox_unchecked.png");
            gameSettingsManager.store();
        }

        if (graphicsManager.needRestart())
        {
            if (_document->isVisible())
            {
                _document->getControlByName("restartInfo")->setVisible(true);
                _document->getControlByName("restart")->setVisible(true);
            }
        }
    }
    if (type == IEventHandler::MouseDown)
    {
        if (control->getName() == "soundSlider")
        {
            _playSound = true;
        }
    }
    if (type == IEventHandler::MouseMove)
    {
        auto& soundManager = Game::getInstance()->getSoundsManager();
        if (control->getName() == "soundSlider")
        {
            auto progress = std::stof(control->getCustomAttribute("progress"));
            soundManager.setSoundVolume(progress);
        } else if(control->getName() == "musicSlider")
        {
            auto progress = std::stof(control->getCustomAttribute("progress"));
            soundManager.setMusicVolume(progress);
        }
    }
}

void SettingsStateProcessor::setGraphicsSettingsValue()
{
    auto graphicsControl = _document->getControlByName("graphics");
    auto settings = Game::getInstance()->getGraphicsManager().getSettings();
    if (settings == _highSettings)
    {
        graphicsControl->setText("High");
    }
    else if (settings == _medSettings)
    {
        graphicsControl->setText("Medium");
    }
    else if (settings == _lowSettings)
    {
        graphicsControl->setText("Low");
    }
    else
    {
        graphicsControl->setText("Custom");
    }
}

} // namespace Chewman