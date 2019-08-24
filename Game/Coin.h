// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "SVE/SceneNode.h"

namespace Chewman
{

struct Coin
{
    std::shared_ptr<SVE::SceneNode> rootNode;
    int type;
};

} // namespace Chewman