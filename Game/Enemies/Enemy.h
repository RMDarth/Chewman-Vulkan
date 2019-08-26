// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>
#include "Game/MapTraveller.h"
#include "Game/GameDefs.h"
#include "EnemyAI.h"

namespace Chewman
{

class Enemy
{
public:
    Enemy(CellInfoMap& map, glm::ivec2 startPos);
    virtual ~Enemy() noexcept = default;

    virtual void update() = 0;

protected:
    CellInfoMap* _map;
    std::shared_ptr<MapTraveller> _mapTraveller;
    std::shared_ptr<EnemyAI> _ai;
};

} // namespace Chewman