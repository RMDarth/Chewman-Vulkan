// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include "GameDefs.h"

namespace Chewman
{

struct GameMap;

enum class MoveDirection : uint8_t
{
    Left,
    Forward,
    Right,
    Backward,
    None
};

class MapTraveller
{
public:
    MapTraveller(GameMap* map, glm::ivec2 startPosMap, float moveSpeed = MoveSpeed);
    MapTraveller(GameMap* map, glm::vec2 startPosReal, float moveSpeed = MoveSpeed);

    bool tryMove(MoveDirection dir);
    void move(MoveDirection dir);
    bool isMovePossible(MoveDirection dir);
    void update(float deltaTime);
    void setSpeed(float speed);
    void setWaterAccessible(bool accessible);

    void setPosition(glm::ivec2 position);

    MoveDirection getCurrentDirection() const;
    glm::ivec2 getMapPosition() const;
    glm::vec2 getRealPosition() const;
    bool isTargetReached() const;

    bool isCloseToAffect(glm::vec2 pos);

    static glm::vec2 toRealPos(glm::ivec2 pos);

private:
    bool isFreePosition(glm::ivec2 position);

private:
    GameMap* _map;

    float _moveSpeed = 0.0f;

    MoveDirection _direction = MoveDirection::Forward;
    glm::vec2 _position  = {};
    glm::vec2 _target = {};
    bool _targetReached = true;
    bool _waterAllowed = false;

    glm::vec2 _speed = {};
};

} // namespace Chewman