// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SVE/SceneNode.h"

namespace Chewman
{

struct GameMap;

enum class PowerUpType : uint8_t
{
    Pentagram,
    Freeze,
    Acceleration,
    Life,
    Bomb,
    Jackhammer, // lower walls
    Teeth       // destroy walls
};

class PowerUp
{
public:
    PowerUp(GameMap* gameMap, glm::ivec2 startPos, char symbolType);
    PowerUp(GameMap* gameMap, glm::ivec2 startPos, PowerUpType type);

    PowerUpType getType() const;

    void update(float deltaTime);
private:
    void rotateItem(float time);

private:
    PowerUpType _type;
    std::shared_ptr<SVE::SceneNode> _rootNode;
};

} // namespace Chewman