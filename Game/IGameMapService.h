// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <glm/vec2.hpp>

namespace Chewman
{

class IGameMapService
{
public:
    virtual void switchDayNight() = 0;
    virtual glm::ivec2 getGameMapSize() = 0;
};

} // namespace Chewman