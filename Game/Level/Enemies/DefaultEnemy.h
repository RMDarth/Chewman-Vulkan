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

class DefaultEnemy : public Enemy
{
public:
    DefaultEnemy(GameMap* map, glm::ivec2 startPos, EnemyType enemyType,
                 const std::string& meshName, std::string normalMaterial,
                 int noReturnWayChance = 85, float lightHeight = 1.5f);

    void update(float deltaTime) override;
    void increaseState(EnemyState state) override;
    void decreaseState(EnemyState state) override;

protected:
    virtual float getHeight();
    void createMaterials();
    void createAppearEffect(glm::mat4 transform);
    void updateDeadState(float deltaTime);

protected:
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _rotateNode;
    std::shared_ptr<SVE::SceneNode> _meshNode;
    std::shared_ptr<SVE::SceneNode> _debuffNode;
    std::shared_ptr<SVE::SceneNode> _appearNode;
    std::shared_ptr<SVE::SceneNode> _appearNodeRotate;
    std::shared_ptr<SVE::MeshEntity> _enemyMesh;

    std::string _normalMaterial;
    std::string _vulnerableMaterial;
    std::string _frostMaterial;
    std::string _frostVulnerableMaterial;

    float _deadTime = -1;
    bool _isAppearing = false;
};

} // namespace Chewman