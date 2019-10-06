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

// path map for single target
using TargetPathMap = std::vector<std::vector<MoveDirection>>;
using PathMapList = std::unordered_map<glm::ivec2, TargetPathMap, GlmHash>;

class HuntAI : public RandomWalkerAI
{
public:
    explicit HuntAI(MapTraveller& mapWalker);

    void update(float deltaTime) override;
    static void updatePathMap(GameMap* map);

private:

    // path map for all possible targets
    static std::unordered_map<glm::ivec2, TargetPathMap, GlmHash> _pathMapList;
};

} // namespace Chewman