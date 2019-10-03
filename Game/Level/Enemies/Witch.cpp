// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Witch.h"
#include "SVE/MeshEntity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <Game/Level/GameMap.h>
#include <SVE/SceneManager.h>

namespace Chewman
{

Witch::Witch(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Nun,
                   "witch", "WitchMaterial", 95)
{
    createMaterials();

    auto transform = glm::scale(glm::mat4(1), glm::vec3(20.0f));
    //transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);

    _castMesh = std::make_shared<SVE::MeshEntity>("witchCast");
    _castMesh->setMaterial(_normalMaterial);

    // Init projectile
    auto projectile = std::make_unique<Projectile>(_gameMap, startPos);
    _projectile = projectile.get();
    _gameMap->enemies.push_back(std::move(projectile));
}

void Witch::update(float deltaTime)
{
    if (_magicRestore > 0)
        _magicRestore -= deltaTime;

    if (_mapTraveller->isTargetReached() && !_projectile->isActive() && isPlayerOnLine() && _castingTime <= 0 && _magicRestore <= 0)
    {
        startMagic(MagicType::Fireball);
    }

    if (_castingTime > 0)
    {
        _castingTime -= deltaTime;
        if (_castingTime < 0.35 && _magicRestore <= 0)
            applyMagic();
        if (_castingTime <= 0)
            stopCasting();
    }
    else
    {
        DefaultEnemy::update(deltaTime);
    }
}

bool Witch::isPlayerOnLine()
{
    auto playerPos = _gameMap->player->getMapTraveller()->getMapPosition();
    auto witchPos = _mapTraveller->getMapPosition();

    if (playerPos.x == witchPos.x)
    {
        for (auto y = std::min(playerPos.y, witchPos.y); y < std::max(playerPos.y, witchPos.y); ++y)
        {
            switch (_gameMap->mapData[witchPos.x][y].cellType)
            {
                case CellType::Wall:
                case CellType::InvisibleWallWithFloor:
                case CellType::InvisibleWallEmpty:
                    return false;
                default:
                    break;
            }
        }

        _magicDirection = playerPos.y > witchPos.y ? MoveDirection::Forward : MoveDirection::Backward;
        return true;
    }
    else if (playerPos.y == witchPos.y)
    {
        for (auto x = std::min(playerPos.x, witchPos.x); x < std::max(playerPos.x, witchPos.x); ++x)
        {
            switch (_gameMap->mapData[x][witchPos.y].cellType)
            {
                case CellType::Wall:
                case CellType::InvisibleWallWithFloor:
                case CellType::InvisibleWallEmpty:
                    return false;
                default:
                    break;
            }
        }

        _magicDirection = playerPos.x > witchPos.x ? MoveDirection::Right : MoveDirection::Left;
        return true;
    }

    return false;
}

void Witch::startMagic(MagicType magicType)
{
    _meshNode->detachEntity(_enemyMesh);
    _magicType = magicType;

    switch (magicType)
    {
        case MagicType::Fireball:
            _meshNode->attachEntity(_castMesh);
            _castMesh->resetTime(0, true);
            _castingTime = 1.2f;
            break;
        case MagicType::Teleport:
            break;
    }

    float rotateAngle = _projectile->getRotateAngle(_magicDirection);
    _rotateNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0)));
}

void Witch::stopCasting()
{
    _meshNode->attachEntity(_enemyMesh);

    switch (_magicType)
    {
        case MagicType::Fireball:
            _meshNode->detachEntity(_castMesh);
            break;
        case MagicType::Teleport:
            break;
    }
}

void Witch::applyMagic()
{
    _magicRestore = 4.0f;

    switch (_magicType)
    {
        case MagicType::Fireball:
            _projectile->activate(_magicDirection, _mapTraveller->getMapPosition(), ProjectileType::Fire);
            break;
        case MagicType::Teleport:
            break;
    }
}

} // namespace Chewman