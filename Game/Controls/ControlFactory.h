// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{

class ControlFactory
{
public:
    std::shared_ptr<Control> createControl(const std::string& type, const std::string& name, float x, float y, float width, float height, std::shared_ptr<Control> parent);
};

} // namespace Chewman