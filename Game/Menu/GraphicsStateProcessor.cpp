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
    : _document(std::make_unique<ControlDocument>("resources/game/GUI/graphics.xml"))
{
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
    auto particleControl = _document->getControlByName("partEffect");
    particleControl->setText(getParticlesText(_settings.particleEffects));

    _document->getControlByName("restartInfo")->setVisible(false);
}

void GraphicsStateProcessor::hide()
{
    _document->hide();
}

bool GraphicsStateProcessor::isOverlapping()
{
    return true;
}

void GraphicsStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Settings);
        }
        if (control->getName() == "accept")
        {
            _document->hide();
            Game::getInstance()->getGraphicsManager().setSettings(_settings);
            Game::getInstance()->setState(GameState::Settings);
        }

        // settings //
        static const std::vector<std::string> shadowValues = { "shadowOff", "shadowOn" };
        static const std::vector<std::string> lightValues = { "lightsOff", "lightsOn" };
        static const std::vector<std::string> resValues = { "resLow", "resHigh" };
        static const std::vector<std::string> effectsValues = { "effectsLow", "effectsHigh" };
        static const std::vector<std::string> gargoyleValues = { "particlesFull", "particlesPartial", "particlesNone" };
        setSettingByName(_settings.useShadows, shadowValues, control->getName());
        setSettingByName(_settings.useDynamicLights, lightValues, control->getName());
        setSettingByName(_settings.effectSettings, effectsValues, control->getName());
        setSettingByName(_settings.resolution, resValues, control->getName());
        setSettingByName(_settings.particleEffects, gargoyleValues, control->getName());

        if (Game::getInstance()->getGraphicsManager().changesRequireRestart(_settings))
        {
            if (_document->isVisible())
                _document->getControlByName("restartInfo")->setVisible(true);
        }

    }
}

} // namespace Chewman