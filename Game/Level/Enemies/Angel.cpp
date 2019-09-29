// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Angel.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Game/Level/GameMap.h"

namespace Chewman
{

Angel::Angel(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Angel,
                   "angel", "AngelMaterial", 85)
{
    createMaterials();

    _mapTraveller->setWaterAccessible(true);
    _mapTraveller->setWallAccessible(true);

    auto transform = glm::scale(glm::mat4(1), glm::vec3(2.0f));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);
}

float Angel::getHeight()
{
    auto startMapPos = _mapTraveller->getMapPosition(_mapTraveller->getStartPos());
    auto targetMapPos = _mapTraveller->getMapPosition(_mapTraveller->getTargetPos());

    auto isWall = [](CellType cellType)
    {
        return cellType == CellType::Wall || cellType == CellType::InvisibleWallEmpty || cellType == CellType::InvisibleWallWithFloor;
    };

    auto getHeight = [](bool isWall)
    {
        if (isWall)
            return 2.0f;
        else
            return 0.0f;
    };

    auto targetIsWall = isWall(_gameMap->mapData[targetMapPos.x][targetMapPos.y].cellType);

    if (isWall(_gameMap->mapData[startMapPos.x][startMapPos.y].cellType) != targetIsWall)
    {
        auto currentPos = _mapTraveller->getMapPosition();
        auto currentIsWall = isWall(_gameMap->mapData[currentPos.x][currentPos.y].cellType);

        if (currentIsWall)
        {
            return 2.0f;
        }
        auto dist = glm::length(_mapTraveller->getRealPosition() - (targetIsWall ? _mapTraveller->getStartPos() : _mapTraveller->getTargetPos()));
        auto step = glm::smoothstep(0.0f, CellSize * 0.5f, dist);
        return 2.0f * step;
    } else {
        return getHeight(targetIsWall);
    }
}

} // namespace Chewman