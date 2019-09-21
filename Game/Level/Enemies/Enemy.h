// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include "Game/Level/MapTraveller.h"
#include "Game/Level/GameMapDefs.h"
#include "EnemyAI.h"

namespace Chewman
{

struct GameMap;

enum class EnemyState
{
    Frozen,
    Vulnerable,
    Dead
};

constexpr size_t EnemyStateCount = 4;

class Enemy
{
public:
    Enemy(GameMap* map, glm::ivec2 startPos);
    virtual ~Enemy() noexcept = default;

    virtual void update(float deltaTime) = 0;
    virtual glm::vec2 getPosition();

    virtual void increaseState(EnemyState state);
    virtual void decreaseState(EnemyState state);
    bool isStateActive(EnemyState state);
    void resetState(EnemyState state);

    bool isDead();

protected:
    GameMap* _gameMap;
    uint8_t _state[EnemyStateCount] = {};
    std::shared_ptr<MapTraveller> _mapTraveller;
    std::shared_ptr<EnemyAI> _ai;
};

} // namespace Chewman