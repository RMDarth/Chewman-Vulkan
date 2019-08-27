// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Enemy.h"
#include "Game/GameMap.h"

namespace Chewman
{

Enemy::Enemy(GameMap* map, glm::ivec2 startPos)
    : _mapTraveller(std::make_shared<MapTraveller>(map, startPos))
{
}

} // namespace Chewman
