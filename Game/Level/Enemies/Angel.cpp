// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Angel.h"
#include "SVE/SceneManager.h"

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

    auto transform = glm::translate(glm::mat4(1), glm::vec3(0, 2.5f, 0));
    transform = glm::scale(transform, glm::vec3(0.01f));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);

    auto wingsNode = SVE::Engine::getInstance()->getSceneManager()->createSceneNode();
    _rotateNode->attachSceneNode(wingsNode);
    transform = glm::translate(glm::mat4(1), glm::vec3(0, 3.75f, 0.31f));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    wingsNode->setNodeTransformation(transform);
    _wingsMesh = std::make_shared<SVE::MeshEntity>("angelWings");
    _wingsMesh->setMaterial("AngelWingsMaterial");
    _wingsMesh->setRenderLast(true);
    _wingsMesh->getMaterialInfo()->ambient = {0.1, 0.1, 0.1, 1.0 };
    wingsNode->attachEntity(_wingsMesh);
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

void Angel::increaseState(EnemyState state)
{
    DefaultEnemy::increaseState(state);
    if (state == EnemyState::Frozen)
    {
        _wingsMesh->setAnimationState(SVE::AnimationState::Pause);
        _wingsMesh->getMaterialInfo()->diffuse = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
    }
}

void Angel::decreaseState(EnemyState state)
{
    DefaultEnemy::decreaseState(state);
    if (state == EnemyState::Frozen)
    {
        _wingsMesh->setAnimationState(SVE::AnimationState::Play);
        _wingsMesh->getMaterialInfo()->diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

} // namespace Chewman