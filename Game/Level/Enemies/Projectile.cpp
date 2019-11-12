// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Projectile.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/ParticleSystemEntity.h"

#include "Game/Game.h"
#include "Game/Level/GameMap.h"
#include "Game/Level/GameUtils.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

Projectile::Projectile(GameMap* map, glm::ivec2 startPos)
    : Enemy(map, startPos, EnemyType::Projectile)
{
    _mapTraveller->setSpeed(MoveSpeed * 2);
    _mapTraveller->setWaterAccessible(true);
    _mapTraveller->setAffectDistance(2.2f);

    auto* engine = SVE::Engine::getInstance();
    _rootNode = engine->getSceneManager()->createSceneNode();
    _rotateNode = engine->getSceneManager()->createSceneNode();
    _psNode = engine->getSceneManager()->createSceneNode();
    _rootNode->attachSceneNode(_rotateNode);
    _rotateNode->attachSceneNode(_psNode);
    _psNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -2.0f)));

    _fireballPS = std::make_shared<SVE::ParticleSystemEntity>("Fireball");
    _frostballPS = std::make_shared<SVE::ParticleSystemEntity>("Frostball");

    if (Game::getInstance()->getGraphicsManager().getSettings().useDynamicLights)
        _rootNode->attachSceneNode(addEnemyLightEffect(engine, 2.0));

    _state[(uint8_t)EnemyState::Dead] = 1;
}

void Projectile::update(float deltaTime)
{
    if (!_isActive)
        return;

    if (_mapTraveller->isTargetReached())
    {
        if (!_mapTraveller->tryMove(_magicDirection))
        {
            increaseState(EnemyState::Dead);
            return;
        }
    }
    _mapTraveller->update(deltaTime);
    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, 2.4f, -realMapPos.x);
    auto transform = glm::translate(glm::mat4(1), position);
    _rootNode->setNodeTransformation(transform);
}

void Projectile::increaseState(EnemyState state)
{
    Enemy::increaseState(state);
    if (state == EnemyState::Dead && _isActive)
    {
        _isActive = false;
        _gameMap->mapNode->detachSceneNode(_rootNode);
    }
}

void Projectile::decreaseState(EnemyState state)
{
    Enemy::decreaseState(state);
    if (state == EnemyState::Dead && !_isActive)
    {
        _isActive = true;
        _gameMap->mapNode->attachSceneNode(_rootNode);
    }
}

bool Projectile::isStateActive(EnemyState state) const
{
    if (state == EnemyState::Vulnerable)
        return false;

    return Enemy::isStateActive(state);
}


bool Projectile::isActive()
{
    return _isActive;
}

void Projectile::activate(MoveDirection direction, glm::ivec2 pos, ProjectileType projectileType)
{
    _mapTraveller->setPosition(pos);
    _magicDirection = direction;
    _type = projectileType;
    _rotateNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(getRotateAngle(direction)), glm::vec3(0, 1, 0)));
    switch (projectileType)
    {
        case ProjectileType::Fire:
            _psNode->detachEntity(_frostballPS);
            _psNode->attachEntity(_fireballPS);
            break;
        case ProjectileType::Frost:
            _psNode->detachEntity(_fireballPS);
            _psNode->attachEntity(_frostballPS);
            break;
    }
    decreaseState(EnemyState::Dead);
}

float Projectile::getRotateAngle(MoveDirection direction)
{
    switch (direction)
    {
        case MoveDirection::Left:
            return 180.0f; // Down
        case MoveDirection::Up:
            return  -90.0f; // Right
        case MoveDirection::Right:
            return 0.0f; // Up
        case MoveDirection::Down:
            return 90.0f; // Left
    }
    assert(!"Invalid direction");
    return 0.0f;
}

ProjectileType Projectile::getProjectileType() const
{
    return _type;
}

void Projectile::resetAll()
{
    Enemy::resetAll();
    increaseState(EnemyState::Dead);
}

} // namespace Chewman