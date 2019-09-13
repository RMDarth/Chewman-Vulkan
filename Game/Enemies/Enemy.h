// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include "Game/MapTraveller.h"
#include "Game/GameDefs.h"
#include "EnemyAI.h"

namespace Chewman
{

struct GameMap;

class Enemy
{
public:
    Enemy(GameMap* map, glm::ivec2 startPos);
    virtual ~Enemy() noexcept = default;

    virtual void update(float deltaTime) = 0;
    virtual glm::vec2 getPosition();

protected:
    std::shared_ptr<MapTraveller> _mapTraveller;
    std::shared_ptr<EnemyAI> _ai;
};

} // namespace Chewman