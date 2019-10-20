// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ImageControl.h"

namespace Chewman
{

ImageControl::ImageControl(const std::string& name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Image, name, x, y, width, height, "empty.png", parent)
{
}

} // namespace Chewman