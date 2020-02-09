// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SettingsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Level/GameUtils.h"
#include "Game/SystemApi.h"
#include "Game/Game.h"


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
    _document->getControlByName("brightSlider")->setMouseMoveHandler(this);
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
    setControllerSettingsValue();

    auto& gameSettings = Game::getInstance()->getGameSettingsManager().getSettings();
    _document->getControlByName("brightSlider")->setCustomAttribute("progress", std::to_string(gameSettings.brightness));

    if (Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::Off)
    {
        _document->getControlByName("brightSlider")->setVisible(false);
        _document->getControlByName("brightLabel")->setVisible(false);
    }

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
        auto& gameSettingsManager = Game::getInstance()->getGameSettingsManager();

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
        else if(control->getName() == "controlsSwipe")
        {
            gameSettingsManager.getSettings().controllerType = ControllerType::Swipe;
            gameSettingsManager.store();
        }
        else if(control->getName() == "controlsJoystick")
        {
            gameSettingsManager.getSettings().controllerType = ControllerType::Joystick;
            gameSettingsManager.store();
        }
        else if(control->getName() == "controlsAccel")
        {
            gameSettingsManager.getSettings().controllerType = ControllerType::Accelerometer;
            if (!System::initAccelerometer())
            {
                _document->getControlByName("controls")->setText("@Swipe");
                gameSettingsManager.getSettings().controllerType = ControllerType::Swipe;
            }
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
        } else if (control->getName() == "brightSlider")
        {
            auto progress = std::stof(control->getCustomAttribute("progress"));
            auto& settingsManager = Game::getInstance()->getGameSettingsManager();
            settingsManager.getSettings().brightness = progress;
            setSunLight(SunLightType::Night);
            settingsManager.store();
        }
    }
}

void SettingsStateProcessor::setGraphicsSettingsValue()
{
    auto graphicsControl = _document->getControlByName("graphics");
    auto settings = Game::getInstance()->getGraphicsManager().getSettings();
    if (settings == _highSettings)
    {
        graphicsControl->setText("@High");
    }
    else if (settings == _medSettings)
    {
        graphicsControl->setText("@Medium");
    }
    else if (settings == _lowSettings)
    {
        graphicsControl->setText("@Low");
    }
    else
    {
        graphicsControl->setText("@Custom");
    }
}

void SettingsStateProcessor::setControllerSettingsValue()
{
    auto controller = _document->getControlByName("controls");
    auto& gameSettings = Game::getInstance()->getGameSettingsManager().getSettings();
    switch (gameSettings.controllerType)
    {
        case ControllerType::Swipe:
            controller->setText("@Swipe");
            break;
        case ControllerType::Joystick:
            controller->setText("@Joystick");
            break;
        case ControllerType::Accelerometer:
            controller->setText("@Accelerometer");
            break;
    }
}

} // namespace Chewman