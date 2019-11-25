// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include "GameMapDefs.h"

namespace Chewman
{

struct GameMap;

enum class MoveDirection : uint8_t
{
    Left,
    Up,
    Right,
    Down,
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
    void setWallAccessible(bool accessible);
    void setAffectDistance(float distance);

    void setPosition(glm::ivec2 position);

    MoveDirection getCurrentDirection() const;
    glm::ivec2 getMapPosition() const;
    static glm::ivec2 getMapPosition(glm::vec2 realPos);
    glm::vec2 getRealPosition() const;
    bool isTargetReached() const;

    glm::vec2 getStartPos() const;
    glm::vec2 getTargetPos() const;
    void setTargetPos(glm::vec2 pos);
    GameMap* getGameMap() const;

    bool isCloseToAffect(glm::vec2 pos) const;

    static glm::vec2 toRealPos(glm::ivec2 pos);

private:
    bool isFreePosition(glm::ivec2 position);

private:
    GameMap* _map = nullptr;

    float _moveSpeed = 0.0f;

    MoveDirection _direction = MoveDirection::Up;
    glm::vec2 _position  = {};
    glm::vec2 _target = {};
    glm::vec2 _start = {};
    bool _targetReached = true;
    bool _waterAllowed = false;
    bool _wallAllowed = false;

    glm::vec2 _speed = {};

    float _affectDistance = 1.3f;
};

} // namespace Chewman