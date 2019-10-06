// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"

namespace Chewman
{

class Knight final : public DefaultEnemy
{
public:
    Knight(GameMap* map, glm::ivec2 startPos);
    void init() override;

    void update(float deltaTime) override;
    void attackPlayer() override;

    void increaseState(EnemyState state) override;

    static void updatePathMap(GameMap* map);
private:
    std::shared_ptr<SVE::SceneNode> _attachmentNode;
    std::shared_ptr<SVE::MeshEntity> _attackMesh;
    float _attackTime = -1;
};

} // namespace Chewman