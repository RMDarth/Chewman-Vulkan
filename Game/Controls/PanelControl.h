// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{

class PanelControl : public Control
{
public:
    PanelControl(const std::string& name, float x, float y, float width, float height, Control* parent = nullptr);
};

} // namespace Chewman