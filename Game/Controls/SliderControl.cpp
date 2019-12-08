// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SliderControl.h"
#include <string>

namespace Chewman
{

SliderControl::SliderControl(const std::string& name, float x, float y, float width, float height, Control* parent)
        : Control(ControlType::Slider, name, x, y, width, height, "slider/groover3.png", parent)
{
    _progressBar = std::make_shared<ImageControl>(name + "ProgressBar", 0.055, 0, 0.905, 1, this);
    _progressBar->setDefaultMaterial("slider/progress2.png");
    _progressBar->setRenderOrder(110);
    _progressMaxWidth = _progressBar->getSize().x;

    _knot = std::make_shared<ImageControl>(name + "Knot", 0.005, 0.03, 0.09, -1, this);
    _knot->setDefaultMaterial("slider/knot.png");
    _knot->setRenderOrder(115);
    _knotStartPos = _knot->getPosition().x;

    setProgress(0.5f);
}

void SliderControl::setCustomAttribute(const std::string& name, std::string value)
{
    if (name == "progress")
    {
        setProgress(std::stof(value));
    }
    else
    {
        Control::setCustomAttribute(name, value);
    }
}

std::string SliderControl::getCustomAttribute(const std::string& name)
{
    if (name == "progress")
        return std::to_string(_progress);
    return Control::getCustomAttribute(name);
}

void SliderControl::setVisible(bool visible)
{
    Control::setVisible(visible);
    _progressBar->setVisible(visible);
    _knot->setVisible(visible);
}

void SliderControl::setProgress(float progress)
{
    _progress = progress;

    _progressBar->setSize({_progressMaxWidth * progress, _progressBar->getSize().y});
    _progressBar->setTexCoords({0.0f, progress, 0.0f, 1.0f});

    _knot->setPosition({_knotStartPos + _progressMaxWidth * progress, _knot->getPosition().y});
}

bool SliderControl::onMouseMove(int x, int y)
{
    if (_slidering)
    {
        auto posInside = x - _knotStartPos - _knot->getSize().x / 2 - 0.005f;
        float progress = 0.0f;
        if (posInside > 0)
        {
            progress = std::min((float)posInside / _progressMaxWidth, 1.0f);
        }

        setProgress(progress);

        std::for_each(_mouseMoveHandlerList.begin(), _mouseMoveHandlerList.end(),
                      [&](IEventHandler* handler) {
                          handler->processEvent(this, IEventHandler::MouseMove, x, y);
                      });

        return true;
    }

    return false;
}

bool SliderControl::onMouseDown(int x, int y)
{
    if (isInside(x, y) && _visible)
    {
        _slidering = true;

        std::for_each(_mouseMoveHandlerList.begin(), _mouseMoveHandlerList.end(),
                      [&](IEventHandler* handler) {
                          handler->processEvent(this, IEventHandler::MouseDown, x, y);
                      });

        return true;
    }

    return false;
}

bool SliderControl::onMouseUp(int x, int y)
{
    if (_slidering)
    {
        _slidering = false;

        std::for_each(_mouseMoveHandlerList.begin(), _mouseMoveHandlerList.end(),
                      [&](IEventHandler* handler) {
                          handler->processEvent(this, IEventHandler::MouseUp, x, y);
                      });

        return true;
    }

    return false;
}

} // namespace Chewman