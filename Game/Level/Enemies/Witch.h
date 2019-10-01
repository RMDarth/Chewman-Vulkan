// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"

namespace Chewman
{

class Witch final : public DefaultEnemy
{
public:
    Witch(GameMap* map, glm::ivec2 startPos);
};

} // namespace Chewman