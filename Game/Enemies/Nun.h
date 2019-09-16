// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/SceneNode.h>
#include <SVE/Engine.h>
#include "Enemy.h"

namespace SVE
{
class MeshEntity;
}

namespace Chewman
{

class Nun final : public Enemy
{
public:
    Nun(GameMap* map, glm::ivec2 startPos);

    void update(float deltaTime) override;
    void increaseState(EnemyState state) override;
    void decreaseState(EnemyState state) override;

private:
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _rotateNode;
    std::shared_ptr<SVE::SceneNode> _debuffNode;
    std::shared_ptr<SVE::MeshEntity> _nunMesh;
    bool isVulnerable = false;
};

} // namespace Chewman