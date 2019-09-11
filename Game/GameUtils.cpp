// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameUtils.h"
#include "GameDefs.h"

namespace Chewman
{

std::mt19937& getRandomEngine()
{
    static std::mt19937 mt(std::random_device{}());
    return mt;
}

glm::vec3 getWorldPos(int row, int column, float y)
{
    return glm::vec3(CellSize * column, y, -CellSize * row);
}

} // namespace Chewman