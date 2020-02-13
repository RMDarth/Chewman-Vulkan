// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ChewmanEnemy.h"
#include "ChewmanAI.h"
#include "Game/Level/GameMap.h"
#include <SVE/MeshEntity.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

ChewmanEnemy::ChewmanEnemy(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Chewman,
            "trashman", "BlueChewmanMaterial",
            95, 2.5f)
{
    _ai = std::make_shared<ChewmanAI>(*_mapTraveller);
    createMaterials();
    _meshNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0, 1, 0)));
}

void ChewmanEnemy::increaseState(EnemyState state)
{
    if (state == EnemyState::Dead && !_gameMap->player->getMapTraveller()->isCloseToAffect(_mapTraveller->getRealPosition()))
    {
        Enemy::increaseState(state);
        _deadTime = ReviveTime;
        _isAppearing = false;

        _enemyMesh->setMaterial("BlueBurnTrashman");
        _enemyMesh->setAnimationState(SVE::AnimationState::Pause);
        _enemyMesh->setRenderLast();
        _enemyMesh->setCastShadows(false);
        _enemyMesh->resetTime(2.2);
        _isDeathAnimation = true;
        _deathAnimation = 1.4f;
        return;
    }

    DefaultEnemy::increaseState(state);
    if (isStateActive(EnemyState::Vulnerable))
        static_cast<ChewmanAI*>(_ai.get())->setVulnerable(true);
}

void ChewmanEnemy::decreaseState(EnemyState state)
{
    DefaultEnemy::decreaseState(state);
    if (!isStateActive(EnemyState::Vulnerable))
        static_cast<ChewmanAI*>(_ai.get())->setVulnerable(false);
}

void ChewmanEnemy::update(float deltaTime)
{
    if (_isDeathAnimation)
    {
        _deathAnimation -= deltaTime;
        if (_deathAnimation < 0)
        {
            _isDeathAnimation = false;
            if (_rootNode->getParent())
                _rootNode->getParent()->detachSceneNode(_rootNode);
        }
    }
    DefaultEnemy::update(deltaTime);
}

void ChewmanEnemy::resetAll()
{
    _isDeathAnimation = false;
    _enemyMesh->setMaterial("BlueChewmanMaterial");
    _enemyMesh->setAnimationState(SVE::AnimationState::Play);
    _enemyMesh->setRenderLast(false);
    _enemyMesh->setCastShadows(true);
    Enemy::resetAll();
}

} // namespace Chewman