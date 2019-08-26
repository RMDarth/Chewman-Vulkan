// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "EnemyAI.h"
#include <random>

namespace Chewman
{

class RandomWalkerAI : public EnemyAI
{
public:
    explicit RandomWalkerAI(MapTraveller& mapWalker, uint8_t sameWayChance = 25);

    void update() override;

private:
    std::mt19937 _randomDevice;
    uint8_t _sameWayChange = 25;
};

} // namespace Chewman