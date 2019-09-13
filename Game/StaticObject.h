// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SVE/SceneNode.h"
#include "GameDefs.h"

namespace Chewman
{
struct GameMap;

enum class StaticObjectType : uint8_t
{
    Tomb,
    Volcano,
    Dragon,
    Pot,
    Mouth
};

class StaticObject
{
public:
    StaticObject(GameMap* gameMap, glm::ivec2 startPos, char symbolType, char rotation);
    StaticObject(GameMap* gameMap, glm::ivec2 startPos, StaticObjectType type, char rotation);

    StaticObjectType getType() const;

    void update(float deltaTime);

    static CellType getCellType(char type);
    static std::pair<size_t, size_t> getSize(char type, char rotation);

private:
    StaticObjectType _type;
    std::shared_ptr<SVE::SceneNode> _rootNode;
};

} // namespace Chewman