// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SVE/SceneNode.h"

namespace Chewman
{

enum class TeleportType
{
    Red,
    Green,
    Blue,
    Purple
};

struct Teleport
{
    std::shared_ptr<SVE::SceneNode> rootNode;

    std::shared_ptr<SVE::SceneNode> circleNode;
    std::shared_ptr<SVE::SceneNode> glowNode;
    glm::ivec2 position;

    Teleport* secondEnd;
    TeleportType type;
};

} // namespace Chewman