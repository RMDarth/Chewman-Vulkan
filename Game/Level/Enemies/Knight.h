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
    bool isCollecting() const;
    void setCollecting(bool value);

    void increaseState(EnemyState state) override;

    void resetAll() override;

    static void updatePathMap(GameMap* map);
private:
    std::shared_ptr<SVE::SceneNode> _attachmentNode;
    std::shared_ptr<SVE::MeshEntity> _attackMesh;
    std::shared_ptr<SVE::MeshEntity> _castMesh;
    float _attackTime = -1;
    float _castTime = 15.0f;
    float _idleTime = -1;
    bool _isCollecting = false;
};

} // namespace Chewman