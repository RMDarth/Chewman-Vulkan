// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Witch.h"
#include "Game/Level/GameMap.h"
#include "Game/Level/GameUtils.h"
#include "Game/Level/CustomEntity.h"
#include "Game/GraphicsSettings.h"
#include "Game/Game.h"

#include "SVE/MeshEntity.h"
#include "SVE/SceneManager.h"

#include <glm/gtc/matrix_transform.hpp>


namespace Chewman
{

Witch::Witch(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Knight,
                   "witch", "WitchMaterial", 95)
{
    createMaterials();

    auto transform = glm::scale(glm::mat4(1), glm::vec3(20.0f));
    //transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);

    _castMesh = std::make_shared<SVE::MeshEntity>("witchCast");
    _castMesh->setMaterial(_normalMaterial);
    _castMesh->getMaterialInfo()->ambient = {0.3, 0.3, 0.3, 1.0 };

    _teleportMesh = std::make_shared<SVE::MeshEntity>("witchCastTeleport");
    _teleportMesh->setMaterial(_normalMaterial);
    _teleportMesh->getMaterialInfo()->ambient = {0.3, 0.3, 0.3, 1.0 };

    if (Game::getInstance()->getGraphicsManager().getSettings().particleEffects == ParticlesSettings::None)
    {
        MagicInfo info {};
        info.color = glm::vec3(1.0, 0.0, 1.0);
        info.maxParticles = 300;
        info.ratio = 15.0;
        info.radius = 1.0f;
        info.particleSize = 0.1;
        _teleportMeshPS = std::make_shared<MagicEntity>("MagicMeshParticleMaterial", info);
        _isParticlesEnabled = false;
    } else
    {
        _teleportPS = std::make_shared<SVE::ParticleSystemEntity>("WitchTeleport");
        _teleportPS->getMaterialInfo()->diffuse = glm::vec4(0.7, 0.3, 1.0, 1.5f);
        _isParticlesEnabled = true;
    }

    // Init projectile
    auto projectile = std::make_unique<Projectile>(_gameMap, startPos);
    _projectile = projectile.get();
    _gameMap->enemies.push_back(std::move(projectile));
}

void Witch::update(float deltaTime)
{
    if (_fireMagicRestore > 0)
        _fireMagicRestore -= deltaTime;
    if (_teleportMagicRestore > 0)
        _teleportMagicRestore -= deltaTime;

    if (isStateActive(EnemyState::Dead))
    {
        DefaultEnemy::update(deltaTime);
        return;
    }

    if (_mapTraveller->isTargetReached())
    {
        if (!_projectile->isActive() && isPlayerOnLine() && _castingTime <= 0 && _fireMagicRestore <= 0)
        {
            startMagic(MagicType::Fireball);
        } else if (_castingTime <= 0 && _fireMagicRestore <= 0 && _teleportMagicRestore <= 0)
        {
            startMagic(MagicType::Teleport);
        }
    }

    if (_castingTime > 0)
    {
        updateMagic(deltaTime);
    }
    else
    {
        DefaultEnemy::update(deltaTime);
    }
}

void Witch::increaseState(EnemyState state)
{
    if (state == EnemyState::Frozen)
    {
        startMagic(MagicType::Defrost);
        return;
    }
    DefaultEnemy::increaseState(state);
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

        _magicDirection = playerPos.y > witchPos.y ? MoveDirection::Up : MoveDirection::Down;
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
    if (_castingTime > 0)
        return;

    _meshNode->detachEntity(_enemyMesh);
    _magicType = magicType;

    switch (magicType)
    {
        case MagicType::Fireball:
        {
            _meshNode->attachEntity(_castMesh);
            _castMesh->resetTime(0, true);
            _castingTime = 1.2f;

            float rotateAngle = _projectile->getRotateAngle(_magicDirection);
            _rotateNode->setNodeTransformation(
                    glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0)));
            break;
        }
        case MagicType::Teleport:
        {
            _meshNode->attachEntity(_teleportMesh);
            _teleportMesh->resetTime(0, true);
            _castingTime = 2.1f;
            break;
        }
        case MagicType::Defrost:
        {
            _meshNode->attachEntity(_teleportMesh);
            _teleportMesh->resetTime(0, true);
            _castingTime = 2.1f;
        }
    }
}

void Witch::updateMagic(float deltaTime)
{
    _castingTime -= deltaTime;
    switch (_magicType)
    {
        case MagicType::Fireball:
        {
            if (_castingTime < 0.35 && _fireMagicRestore <= 0)
                applyMagic();
            if (_castingTime <= 0)
                stopCasting();
            break;
        }
        case MagicType::Teleport:
        {
            if (_castingTime < 1.4 && !_teleportPSAttached)
            {
                _teleportPSAttached = true;
                if (_isParticlesEnabled)
                {
                    _teleportPS->getMaterialInfo()->diffuse = glm::vec4(0.7, 0.3, 1.0, 1.5f);
                    _rootNode->attachEntity(_teleportPS);
                } else {
                    _teleportMeshPS->getInfo().color = glm::vec3(1.0, 0.2, 1.0);
                    _rootNode->attachEntity(_teleportMeshPS);
                }
            }
            if (_castingTime < 0.6 && _teleportMagicRestore <= 0)
                applyMagic();
            if (_castingTime <= 0)
                stopCasting();
            break;
        }

        case MagicType::Defrost:
        {
            if (_castingTime < 1.4 && !_teleportPSAttached)
            {
                _teleportPSAttached = true;
                if (_isParticlesEnabled)
                {
                    _teleportPS->getMaterialInfo()->diffuse = glm::vec4(0.3, 0.3, 1.0, 1.5f);
                    _rootNode->attachEntity(_teleportPS);
                } else {
                    _teleportMeshPS->getInfo().color = glm::vec3(0.3f, 0.3f, 1.0f);
                    _rootNode->attachEntity(_teleportMeshPS);
                }
            }
            if (_castingTime <= 0)
                stopCasting();
            break;
        }
    }
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
            _meshNode->detachEntity(_teleportMesh);
            if (_isParticlesEnabled)
                _rootNode->detachEntity(_teleportPS);
            else
                _rootNode->detachEntity(_teleportMeshPS);
            _teleportPSAttached = false;
            break;
        case MagicType::Defrost:
            _meshNode->detachEntity(_teleportMesh);
            if (_isParticlesEnabled)
                _rootNode->detachEntity(_teleportPS);
            else
                _rootNode->detachEntity(_teleportMeshPS);
            _teleportPSAttached = false;
            break;
    }
}

void Witch::applyMagic()
{
    auto& randomEngine = getRandomEngine();
    switch (_magicType)
    {
        case MagicType::Fireball:
        {
            _fireMagicRestore = 4.0f;
            auto type = std::uniform_int_distribution<>(0, 4)(randomEngine) < 2 ? ProjectileType::Frost : ProjectileType::Fire;
            _projectile->activate(_magicDirection, _mapTraveller->getMapPosition(), type);
            break;
        }
        case MagicType::Teleport:
        {
            _teleportMagicRestore = std::uniform_real_distribution<float>(6.0f, 14.0f)(randomEngine);
            auto playerPos = _gameMap->player->getMapTraveller()->getMapPosition();
            int x, y;
            do
            {
                x = std::uniform_int_distribution<>(0, _gameMap->height - 1)(randomEngine);
                y = std::uniform_int_distribution<>(0, _gameMap->width - 1)(randomEngine);
            } while(_gameMap->mapData[x][y].cellType != CellType::Floor || (abs(playerPos.x - x) < 2 && abs(playerPos.y - y) < 2));
            _mapTraveller->setPosition({x, y});
            DefaultEnemy::update(0);
            break;
        }
    }
}

void Witch::resetAll()
{
    Enemy::resetAll();
    if (_castingTime > 0)
        stopCasting();
    _castingTime = -1;
    _fireMagicRestore = -1;
    _teleportMagicRestore = std::uniform_real_distribution<float>(6.0f, 14.0f)(getRandomEngine());
}

} // namespace Chewman