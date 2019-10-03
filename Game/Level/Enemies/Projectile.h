// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/SceneNode.h>
#include <SVE/Engine.h>
#include "Enemy.h"

namespace Chewman
{

enum class ProjectileType
{
    Fire,
    Frost
};

class Projectile : public Enemy
{
public:
    Projectile(GameMap* map, glm::ivec2 startPos);

    bool isActive();
    void activate(MoveDirection direction, glm::ivec2 pos, ProjectileType type);

    void update(float deltaTime) override;
    void increaseState(EnemyState state) override;
    void decreaseState(EnemyState state) override;

    static float getRotateAngle(MoveDirection direction);

protected:
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _rotateNode;
    MoveDirection _magicDirection = MoveDirection::None;
    ProjectileType _type = ProjectileType::Fire;
    bool _isActive = false;
};

} // namespace Chewman