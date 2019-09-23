// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{

class ButtonControl : public Control
{
public:
    ButtonControl(std::string name, float x, float y, float width, float height, Control* parent = nullptr);

    bool onMouseMove(int x, int y, float deltaTime) override;
    bool onMouseDown(int x, int y) override;
};

} // namespace Chewman