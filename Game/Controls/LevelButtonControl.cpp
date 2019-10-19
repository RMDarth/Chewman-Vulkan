// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LevelButtonControl.h"

namespace Chewman
{

glm::vec4 LevelButtonControl::_defaultPressedColor = { 1.0f, 1.0f, 1.0f, 1.0f };
glm::vec4 LevelButtonControl::_defaultHoverColor = { 1.0f, 1.0f, 1.0f, 1.0f };
glm::vec4 LevelButtonControl::_defaultNormalColor = { 0.75f, 0.75f, 0.75f, 1.0f };

LevelButtonControl::LevelButtonControl(const std::string& name, float x, float y, float width, float height, Control* parent)
        : Control(ControlType::LevelButton, name, x, y, width, height, "buttons/level0.png", parent)
          , _normalFontColor(_defaultNormalColor)
          , _hoverFontColor(_defaultHoverColor)
          , _pressedFontColor(_defaultPressedColor)
{
    setTextColor(_normalFontColor);
}

bool LevelButtonControl::onMouseMove(int x, int y)
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

bool LevelButtonControl::onMouseDown(int x, int y)
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

void LevelButtonControl::setStarsNum(const std::string& num)
{
    setDefaultMaterial("buttons/level" + num + ".png");
}

void LevelButtonControl::setCustomAttribute(const std::string& name, std::string value)
{
    if (name == "stars")
    {
        setStarsNum(value);
    } else
    {
        Control::setCustomAttribute(name, value);
    }
}

std::string LevelButtonControl::getCustomAttribute(const std::string& name)
{
    return Control::getCustomAttribute(name);
}


} // namespace Chewman