// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"

namespace Chewman
{

class Nun final : public DefaultEnemy
{
public:
    Nun(GameMap* map, glm::ivec2 startPos);
};

} // namespace Chewman