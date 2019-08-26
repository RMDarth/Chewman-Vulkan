// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include "GameDefs.h"

namespace Chewman
{

enum class MoveDirection : uint8_t
{
    Left,
    Right,
    Forward,
    Backward
};

class MapTraveller
{
public:
    MapTraveller(CellInfoMap& map, glm::ivec2 startPosMap, float moveSpeed = MoveSpeed);
    MapTraveller(CellInfoMap& map, glm::vec2 startPosReal, float moveSpeed = MoveSpeed);

    bool tryMove(MoveDirection dir);
    void move(MoveDirection dir);
    bool isMovePossible(MoveDirection dir);
    void update();
    void setSpeed(float speed);
    void setWaterAccessible(bool accessible);

    MoveDirection getCurrentDirection() const;
    glm::ivec2 getMapPosition() const;
    glm::vec2 getRealPosition() const;
    bool isTargetReached() const;

private:
    bool isFreePosition(glm::ivec2 position);

private:
    CellInfoMap* _map;

    float _moveSpeed = 0.0f;

    MoveDirection _direction = MoveDirection::Forward;
    glm::vec2 _position  = {};
    glm::vec2 _target = {};
    bool _targetReached = true;
    bool _waterAllowed = false;

    glm::vec2 _speed = {};
};

} // namespace SVE