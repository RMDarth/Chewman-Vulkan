// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameUtils.h"

namespace Chewman
{

std::mt19937& getRandomEngine()
{
    static std::mt19937 mt(std::random_device{}());
    return mt;
}

} // namespace Chewman