// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Knight.h"
#include "HuntAI.h"
#include "Game/Level/GameMap.h"
#include "SVE/SceneManager.h"
#include "SVE/MeshEntity.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

Knight::Knight(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Knight,
                   "knight", "KnightMaterial", 95)
{
    _ai = std::make_shared<HuntAI>(*_mapTraveller);
    createMaterials();

    auto transform = glm::scale(glm::mat4(1), glm::vec3(1.5f));
    _meshNode->setNodeTransformation(transform);

    auto* engine = SVE::Engine::getInstance();

    _attachmentNode = engine->getSceneManager()->createSceneNode();
    _meshNode->attachSceneNode(_attachmentNode);
    _attachmentNode->setEntityAttachment(_enemyMesh, "mount0");
    _attachmentNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.1f)));

    auto sword = std::make_shared<SVE::MeshEntity>("sword");
    sword->setMaterial("ShortSwordMaterial");
    _attachmentNode->attachEntity(sword);

    _attackMesh = std::make_shared<SVE::MeshEntity>("knightAttack");
    _attackMesh->setMaterial(_normalMaterial);
    _attackMesh->getMaterialInfo()->ambient = {0.3, 0.3, 0.3, 1.0 };

    _castMesh = std::make_shared<SVE::MeshEntity>("knightCast");
    _castMesh->setMaterial(_normalMaterial);
    _castMesh->getMaterialInfo()->ambient = {0.3, 0.3, 0.3, 1.0 };
}

void Knight::updatePathMap(GameMap* map)
{
    HuntAI::updatePathMap(map);
}

void Knight::init()
{
    Enemy::init();
    for (auto& enemy : _gameMap->enemies)
    {
        if (enemy->getEnemyType() == EnemyType::Knight)
        {
            if (enemy.get() == this)
                updatePathMap(_gameMap);
            break;
        }
    }
}

void Knight::update(float deltaTime)
{
    if (_attackTime > 0)
    {
        _attackTime -= deltaTime;
        if (_attackTime <= 0)
        {
            _meshNode->attachEntity(_enemyMesh);
            _meshNode->detachEntity(_attackMesh);
            _attachmentNode->setEntityAttachment(_enemyMesh, "mount0");
        }
    } else if (!isDead())
    {
        if (_idleTime > 0)
        {
            _idleTime -= deltaTime;
            if (_idleTime < 3.5)
            {
                _isCollecting = true;
            }
            if (_idleTime < 0)
            {
                _meshNode->attachEntity(_enemyMesh);
                _meshNode->detachEntity(_castMesh);
                _attachmentNode->setEntityAttachment(_enemyMesh, "mount0");
                _castTime = 15.0f;

                auto transform = glm::scale(glm::mat4(1), glm::vec3(1.5f));
                _meshNode->setNodeTransformation(transform);
            }
        } else
        {
            _castTime -= deltaTime;
            if (_castTime <= 0)
            {
                _idleTime = 5.0f;
                _meshNode->attachEntity(_castMesh);
                _meshNode->detachEntity(_enemyMesh);
                _attachmentNode->setEntityAttachment(_castMesh, "mount0");

                auto transform = glm::scale(glm::mat4(1), glm::vec3(1.5f));
                transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
                _meshNode->setNodeTransformation(transform);
            }
            DefaultEnemy::update(deltaTime);
        }
    } else {
        DefaultEnemy::update(deltaTime);
    }
}

void Knight::attackPlayer()
{
    Enemy::attackPlayer();
    _meshNode->attachEntity(_attackMesh);
    _meshNode->detachEntity(_enemyMesh);
    _attackMesh->resetTime();

    _attachmentNode->setEntityAttachment(_attackMesh, "mount0");

    _attackTime = 0.1f;
}

void Knight::increaseState(EnemyState state)
{
    if (state == EnemyState::Vulnerable)
        return;

    DefaultEnemy::increaseState(state);
}

bool Knight::isCollecting() const
{
    return _isCollecting;
}

void Knight::setCollecting(bool value)
{
    _isCollecting = value;
}

void Knight::resetAll()
{
    Enemy::resetAll();
    _meshNode->attachEntity(_enemyMesh);
    _meshNode->detachEntity(_attackMesh);
    _meshNode->detachEntity(_castMesh);
    _attachmentNode->setEntityAttachment(_enemyMesh, "mount0");

    auto transform = glm::scale(glm::mat4(1), glm::vec3(1.5f));
    _meshNode->setNodeTransformation(transform);

    _castTime = 15.0;
    _attackTime = -1;
    _idleTime = -1;
    _isCollecting = false;
}

} // namespace Chewman