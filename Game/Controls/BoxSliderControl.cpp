// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "BoxSliderControl.h"

namespace Chewman
{
namespace
{

void alignToZero(float& value, float deltaTime, float alignSpeed)
{
    if (value > 0.01)
    {
        value -= deltaTime * alignSpeed;
        if (value < 0)
            value = 0;
    } else if (value < -0.01)
    {
        value += deltaTime * alignSpeed;
        if (value > 0)
            value = 0;
    }
}

} // anon namespace

BoxSliderControl::BoxSliderControl(const std::string& name, float x, float y, float width, float height, Control* parent)
        : Control(ControlType::BoxSlider, name, x, y, width, height, std::string(), parent)
        , _halfWidth(_width / 2)
        , _nextObjShift(_halfWidth + _halfWidth / 2)
{
    setRenderOrder(50);
}

void BoxSliderControl::update(float deltaTime)
{
    _nextObjShift = _children.front()->getSize().x + _halfWidth / 8;
    for (auto i = 0; i < _children.size(); ++i)
    {
        int slideShift = _width/2 + i * _nextObjShift - _children[i]->getSize().x / 2; // base shift
        slideShift -= _currentObject * _nextObjShift;

        if (_isMoving)
            slideShift += _currentShift - _startShift;
        else
        {
            slideShift -= _remainShift;
            alignToZero(_remainShift, deltaTime, _width);
        }

        _children[i]->setPosition(glm::ivec2(slideShift, (_height - _children[i]->getSize().y) / 2));
    }
    Control::update(deltaTime);
}

bool BoxSliderControl::onMouseMove(int x, int y)
{
    if (isInside(x, y) && _isMoving && !_mouseTransparent)
    {
        _currentShift = x;
        return true;
    }

    return false;
}

bool BoxSliderControl::onMouseDown(int x, int y)
{
    if (isInside(x, y) && _visible && !_mouseTransparent)
    {
        _isMoving = true;
        _startShift = x + _remainShift;
        _remainShift = 0;
        _currentShift = x;

        return true;
    }
    return false;
}

bool BoxSliderControl::onMouseUp(int x, int y)
{
    if (!_visible)
        return Control::onMouseUp(x, y);

    if (_isMoving)
    {
        _isMoving = false;
        _remainShift = _startShift - _currentShift;

        if (fabsf(_remainShift) < _width * 0.03)
        {
            // just click
            if (_children[_currentObject]->isInside(x, y))
            {
                std::for_each(_mouseUpHandlerList.begin(), _mouseUpHandlerList.end(),
                              [&](IEventHandler* handler) { handler->processEvent(this, IEventHandler::MouseUp, x, y); });
            }
        }
        else if (_startShift - _currentShift > _nextObjShift * 0.3f && _currentObject < _children.size() - 1)
        {
            // shifting to right object
            _currentObject++;
            _remainShift -= _nextObjShift;
        }
        else if (_currentShift - _startShift > _nextObjShift * 0.3f && _currentObject > 0)
        {
            // shifting to left object
            _currentObject--;
            _remainShift += _nextObjShift;
        }

        return true;
    }

    return false;
}

uint32_t BoxSliderControl::getSelectedObject() const
{
    return _currentObject;
}

} // namespace Chewman