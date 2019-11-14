// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "PanelControl.h"

namespace Chewman
{

PanelControl::PanelControl(const std::string& name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Panel, name, x, y, width, height, "windows/window.png", parent)
{
}

} // namespace Chewman