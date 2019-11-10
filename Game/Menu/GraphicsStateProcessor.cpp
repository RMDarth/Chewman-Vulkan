// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GraphicsStateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <Game/Game.h>

namespace Chewman
{
namespace
{

std::string boolToText(bool value)
{
    return value ? "On" : "Off";
}

template <typename T>
void setSettingByName(T& setting, const std::vector<std::string>& values, const std::string& currentValue)
{
    for (auto i = 0; i < values.size(); ++i)
    {
        if (values[i] == currentValue)
        {
            setting = static_cast<T>(i);
        }
    }
}

} // anon namespace

GraphicsStateProcessor::GraphicsStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/graphics.xml");
    _document->setMouseUpHandler(this);
    _document->hide();
}

GraphicsStateProcessor::~GraphicsStateProcessor() = default;

GameState GraphicsStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::Graphics;
}

void GraphicsStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void GraphicsStateProcessor::show()
{
    _document->show();
    _settings = Game::getInstance()->getGraphicsManager().getSettings();

    auto resolutionControl = _document->getControlByName("resolution");
    resolutionControl->setText(getResolutionText(_settings.resolution));
    auto effectsControl = _document->getControlByName("effects");
    effectsControl->setText(getEffectText(_settings.effectSettings));
    auto lightsControl = _document->getControlByName("lights");
    lightsControl->setText(boolToText(_settings.useDynamicLights));
    auto shadowsControl = _document->getControlByName("shadows");
    shadowsControl->setText(boolToText(_settings.useShadows));
    auto gargoyleControl = _document->getControlByName("gargFireEffect");
    gargoyleControl->setText(getGargoyleText(_settings.gargoyleEffects));

    _document->getControlByName("restartInfo")->setVisible(false);
}

void GraphicsStateProcessor::hide()
{
    _document->hide();
}

bool GraphicsStateProcessor::isOverlapping()
{
    return false;
}

void GraphicsStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            Game::getInstance()->setState(GameState::MainMenu);
        }
        if (control->getName() == "accept")
        {
            Game::getInstance()->getGraphicsManager().setSettings(_settings);
            Game::getInstance()->setState(GameState::MainMenu);
        }

        // settings //
        static const std::vector<std::string> shadowValues = { "shadowOff", "shadowOn" };
        static const std::vector<std::string> lightValues = { "lightsOff", "lightsOn" };
        static const std::vector<std::string> resValues = { "resLow", "resHigh" };
        static const std::vector<std::string> effectsValues = { "effectsLow", "effectsHigh" };
        static const std::vector<std::string> gargoyleValues = { "gargParticles", "gargMesh" };
        setSettingByName(_settings.useShadows, shadowValues, control->getName());
        setSettingByName(_settings.useDynamicLights, lightValues, control->getName());
        setSettingByName(_settings.effectSettings, effectsValues, control->getName());
        setSettingByName(_settings.resolution, resValues, control->getName());
        setSettingByName(_settings.gargoyleEffects, gargoyleValues, control->getName());

        auto currentSettings = Game::getInstance()->getGraphicsManager().getSettings();
        if (_settings.effectSettings != currentSettings.effectSettings
            || _settings.resolution != currentSettings.resolution)
        {
            _document->getControlByName("restartInfo")->setVisible(true);
        }

    }
}

} // namespace Chewman