// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ContainerControl.h"

namespace Chewman
{

ContainerControl::ContainerControl(const std::string& name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Container, name, x, y, width, height, std::string(), parent)
{
    setRenderOrder(50);
}
} // namespace Chewman