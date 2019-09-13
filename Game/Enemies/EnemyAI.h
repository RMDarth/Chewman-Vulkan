// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Game/MapTraveller.h"

namespace Chewman
{

class EnemyAI
{
public:
    EnemyAI(MapTraveller& mapWalker)
        : _mapTraveller (&mapWalker)
    {
    }
    virtual ~EnemyAI() = default;

    virtual void update(float deltaTime) = 0;

protected:
    MapTraveller* _mapTraveller;
};

} // namespace Chewman