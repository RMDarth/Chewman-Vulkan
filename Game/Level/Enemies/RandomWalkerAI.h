// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "EnemyAI.h"
#include <random>

namespace Chewman
{

class RandomWalkerAI : public EnemyAI
{
public:
    explicit RandomWalkerAI(MapTraveller& mapWalker, uint8_t noReturnWayChance = 75);

    void update(float deltaTime) override;

private:
    uint8_t _noReturnWayChance = 75;
};

} // namespace Chewman