// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ButtonControl.h"

namespace Chewman
{

glm::vec4 ButtonControl::_defaultPressedColor = { 0.082f, 0.925f, 0.976f, 1.0f };
glm::vec4 ButtonControl::_defaultHoverColor = { 1.0f, 1.0f, 1.0f, 1.0f };
glm::vec4 ButtonControl::_defaultNormalColor = { 0.086f, 0.925f, 0.976f, 1.0f };

ButtonControl::ButtonControl(std::string name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Button, std::move(name), x, y, width, height, "buttons/button_normal.png", parent)
    , _normalFontColor(_defaultNormalColor)
    , _hoverFontColor(_defaultHoverColor)
    , _pressedFontColor(_defaultPressedColor)
{
    setHoverMaterial("buttons/button_hover.png");
    setPushMaterial("buttons/button_pressed.png");

    setTextColor(_normalFontColor);
}

bool ButtonControl::onMouseMove(int x, int y)
{
    auto result = Control::onMouseMove(x, y);
    if (result && !_text.empty())
    {
        if (_pressed)
            setTextColor(_pressedFontColor);
        else
            setTextColor(_hoverFontColor);
        _isFontColorChanged = true;
    }
    else if (_isFontColorChanged)
    {
        setTextColor(_normalFontColor);
    }

    return result;
}

bool ButtonControl::onMouseDown(int x, int y)
{
    bool result = Control::onMouseDown(x, y);
    if (result)
    {
        setTextColor(_pressedFontColor);
        _isFontColorChanged = true;
    }
    else if (_isFontColorChanged)
    {
        setTextColor(_normalFontColor);
    }

    return result;
}
} // namespace Chewman