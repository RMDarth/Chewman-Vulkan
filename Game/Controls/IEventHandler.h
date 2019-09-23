// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>

namespace Chewman
{

class Control;

class IEventHandler
{
public:
    enum EventType
    {
        MouseUp,
        MouseDown,
        MouseMove
    };

    virtual void ProcessEvent(Control* control, EventType type, int x, int y) = 0;
};

} // namespace Chewman