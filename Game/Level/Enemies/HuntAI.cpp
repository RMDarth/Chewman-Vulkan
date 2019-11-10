// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "HuntAI.h"
#include "Game/Level/GameMap.h"

namespace Chewman
{
namespace
{

TargetPathMap createPathMapForTarget(glm::ivec2 target, GameMap* gameMap)
{
    TargetPathMap pathMap;
    std::vector<std::vector<int>> stepMap;

    const auto initMapSize = [&gameMap](auto& map, auto initValue)
    {
        map.resize(gameMap->height);
        for (auto& row : map)
            row.resize(gameMap->width, initValue);
    };

    initMapSize(pathMap, MoveDirection::None);
    initMapSize(stepMap, -1);

    stepMap[target.x][target.y] = 0;
    int currentStep = 0;
    bool changes = true;

    auto tryGoCell = [&](int x, int y)
    {
        if (x >= 0 && x < gameMap->height && y >= 0 && y < gameMap->width)
        {
            if (gameMap->mapData[x][y].cellType == CellType::Floor && stepMap[x][y] < 0)
            {
                changes = true;
                stepMap[x][y] = currentStep + 1;
            }
        }
    };
    auto isCorrectDirection = [&](int x, int y, int step) -> bool
    {
        if (x >= 0 && x < gameMap->height && y >= 0 && y < gameMap->width)
        {
            return stepMap[x][y] >= 0 && stepMap[x][y] == step - 1;
        }
        return false;
    };

    // Simple BFS map traversal
    while (changes)
    {
        changes = false;
        for (int x = 0; x < gameMap->height; ++x)
        {
            for (int y = 0; y < gameMap->width; ++y)
            {
                if (stepMap[x][y] == currentStep)
                {
                    tryGoCell(x+1, y);
                    tryGoCell(x-1, y);
                    tryGoCell(x, y+1);
                    tryGoCell(x, y-1);
                }
            }
        }
        ++currentStep;
    }

    for (int x = 0; x < gameMap->height; ++x)
    {
        for (int y = 0; y < gameMap->width; ++y)
        {
            auto step = stepMap[x][y];
            if (step == 0)
                continue;

            if (isCorrectDirection(x+1, y, step))
                pathMap[x][y] = MoveDirection::Right;
            else if (isCorrectDirection(x-1, y, step))
                pathMap[x][y] = MoveDirection::Left;
            else if (isCorrectDirection(x, y+1, step))
                pathMap[x][y] = MoveDirection::Up;
            else if (isCorrectDirection(x, y-1, step))
                pathMap[x][y] = MoveDirection::Down;
        }
    }

    return pathMap;
}

} // anon namespace

std::unordered_map<glm::ivec2, TargetPathMap, GlmHash> HuntAI::_pathMapList = {};

HuntAI::HuntAI(MapTraveller& mapWalker)
    : RandomWalkerAI(mapWalker)
{
}

void HuntAI::update(float deltaTime)
{
    if (_mapTraveller->isTargetReached())
    {
        auto target = _mapTraveller->getGameMap()->player->getMapTraveller()->getMapPosition();
        auto pathIter = _pathMapList.find(target);
        if (pathIter != _pathMapList.end())
        {
            auto pathMap = pathIter->second;
            auto currentPos = _mapTraveller->getMapPosition();
            auto direction = pathMap[currentPos.x][currentPos.y];
            if (currentPos.x - target.x <= 7 && currentPos.y - target.y <= 7 && direction != MoveDirection::None)
            {
                if (_mapTraveller->tryMove(direction))
                    return;
                assert(!"Invalid pathmap");
            }
        }
    }
    RandomWalkerAI::update(deltaTime);
}

void HuntAI::updatePathMap(GameMap* map)
{
    _pathMapList.clear();

    for (auto x = 0; x < map->height; ++x)
    {
        for (auto y = 0; y < map->width; ++y)
        {
            if (map->mapData[x][y].cellType == CellType::Floor)
            {
                auto target = glm::ivec2(x, y);
                _pathMapList.emplace(target, createPathMapForTarget(target, map));
            }
        }
    }
}

} // namespace Chewman