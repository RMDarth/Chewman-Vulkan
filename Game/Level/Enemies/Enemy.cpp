// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Enemy.h"
#include "Game/Level/GameMap.h"

namespace Chewman
{

Enemy::Enemy(GameMap* map, glm::ivec2 startPos, EnemyType enemyType)
    : _gameMap(map)
    , _mapTraveller(std::make_shared<MapTraveller>(map, startPos))
    , _enemyType(enemyType)
{
}

void Enemy::init()
{
}

glm::vec2 Enemy::getPosition()
{
    return _mapTraveller->getRealPosition();
}

void Enemy::increaseState(EnemyState state)
{
    ++_state[(uint8_t)state];
}

void Enemy::decreaseState(EnemyState state)
{
    assert(_state[(uint8_t)state]);
    --_state[(uint8_t)state];
}

bool Enemy::isStateActive(EnemyState state) const
{
    return _state[(uint8_t)state];
}

bool Enemy::isDead() const
{
    return isStateActive(EnemyState::Dead);
}

void Enemy::resetState(EnemyState state)
{
    while (isStateActive(state))
        decreaseState(state);
}

void Enemy::attackPlayer()
{
}

EnemyType Enemy::getEnemyType() const
{
    return _enemyType;
}

std::shared_ptr<MapTraveller> Enemy::getMapTraveller()
{
    return _mapTraveller;
}

} // namespace Chewman
