// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"
#include "Projectile.h"

namespace Chewman
{

class Witch final : public DefaultEnemy
{
public:
    Witch(GameMap* map, glm::ivec2 startPos);

    void update(float deltaTime) override;

private:
    enum class MagicType
    {
        Fireball,
        Teleport
    };

    bool isPlayerOnLine();
    void startMagic(MagicType magicType);
    void applyMagic();
    void stopCasting();

private:
    Projectile* _projectile;

    float _castingTime = -1.0f;
    MagicType _magicType = MagicType::Fireball;
    MoveDirection _magicDirection;
    std::shared_ptr<SVE::MeshEntity> _castMesh;

    float _magicRestore = 0.0;
};

} // namespace Chewman