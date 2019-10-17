// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LabelControl.h"

namespace Chewman
{

LabelControl::LabelControl(const std::string& name, float x, float y, float width, float height, Control* parent)
    : Control(ControlType::Label, name, x, y, width, height, "empty.png", parent)
{

}
} // namespace Chewman