// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SettingsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{

const GraphicsSettings _highSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::High, true, true, ParticlesSettings::Partial, EffectSettings::High};
const GraphicsSettings _medSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::High, true, false, ParticlesSettings::Partial, EffectSettings::Low};
const GraphicsSettings _lowSettings = { CurrentGraphicsSettingsVersion, ResolutionSettings::Low, true, false, ParticlesSettings::Partial, EffectSettings::Low};

SettingsStateProcessor::SettingsStateProcessor()
        : _document(std::make_unique<ControlDocument>("resources/game/GUI/settings.xml"))
{
    _document->setMouseUpHandler(this);

    _document->hide();
}

SettingsStateProcessor::~SettingsStateProcessor() = default;

GameState SettingsStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::Settings;
}

void SettingsStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void SettingsStateProcessor::show()
{
    _document->show();

    bool needRestart = Game::getInstance()->getGraphicsManager().needRestart();
    _document->getControlByName("restartInfo")->setVisible(needRestart);
    _document->getControlByName("restart")->setVisible(needRestart);

    setGraphicsSettingsValue();
}

void SettingsStateProcessor::hide()
{
    _document->hide();
    _document->getControlByName("restartInfo")->setVisible(false);
    _document->getControlByName("restart")->setVisible(false);
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

        GraphicsSettings oldSettings = graphicsManager.getSettings();
        if (control->getName() == "graphicsHigh")
        {
            graphicsManager.setSettings(_highSettings);
        }

        if (control->getName() == "graphicsMed")
        {
            graphicsManager.setSettings(_medSettings);
        }

        if (control->getName() == "graphicsLow")
        {
            graphicsManager.setSettings(_lowSettings);
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