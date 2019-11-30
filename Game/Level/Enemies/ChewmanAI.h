// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "RandomWalkerAI.h"
#include "Game/Level/GameUtils.h"
#include <vector>
#include <unordered_map>

namespace Chewman
{

class ChewmanAI : public RandomWalkerAI
{
public:
    explicit ChewmanAI(MapTraveller& mapWalker);

    void update(float deltaTime) override;
    void setVulnerable(bool value);

private:
    bool _isVulnerable = false;
};

} // namespace Chewman