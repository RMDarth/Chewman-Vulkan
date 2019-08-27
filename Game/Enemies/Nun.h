// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <SVE/SceneNode.h>
#include <SVE/Engine.h>
#include "Enemy.h"

namespace Chewman
{

class Nun final : public Enemy
{
public:
    Nun(GameMap* map, glm::ivec2 startPos);

    void update() override;
private:
    std::shared_ptr<SVE::SceneNode> _rootNode;
    bool isVulnerable = false;
};

} // namespace Chewman