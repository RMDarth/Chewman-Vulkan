// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Enemy.h"

namespace Chewman
{

Enemy::Enemy(CellInfoMap& map, glm::ivec2 startPos)
    : _map(&map)
    , _mapTraveller(std::make_shared<MapTraveller>(map, startPos))
{
}

} // namespace Chewman
