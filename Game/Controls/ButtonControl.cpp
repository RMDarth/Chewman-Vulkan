// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ButtonControl.h"

namespace Chewman
{

ButtonControl::ButtonControl(std::string name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Button, std::move(name), x, y, width, height, "buttons/button_normal.png", parent)
{
    setHoverMaterial("buttons/button_hover.png");
    setPushMaterial("buttons/button_pressed.png");
}

bool ButtonControl::onMouseMove(int x, int y, float deltaTime)
{
    // TODO: Add font color change
    return Control::onMouseMove(x, y, deltaTime);
}

bool ButtonControl::onMouseDown(int x, int y)
{
    return Control::onMouseDown(x, y);
}
} // namespace Chewman