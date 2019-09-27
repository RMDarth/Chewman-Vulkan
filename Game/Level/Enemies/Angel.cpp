// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Angel.h"

#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/MeshEntity.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Game/Level/GameMap.h"
#include "Game/Level/GameUtils.h"
#include "RandomWalkerAI.h"

namespace Chewman
{

Angel::Angel(GameMap* map, glm::ivec2 startPos)
    : Enemy(map, startPos)
{
    _ai = std::make_shared<RandomWalkerAI>(*_mapTraveller, 85);
    _mapTraveller->setWaterAccessible(true);
    _mapTraveller->setWallAccessible(true);

    auto* engine = SVE::Engine::getInstance();

    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    _rotateNode = engine->getSceneManager()->createSceneNode();
    _rootNode->attachSceneNode(_rotateNode);

    _rootNode->setNodeTransformation(transform);
    map->mapNode->attachSceneNode(_rootNode);

    auto meshNode = engine->getSceneManager()->createSceneNode();
    auto tranform = glm::scale(glm::mat4(1), glm::vec3(2.0f));
    transform = glm::rotate(tranform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    meshNode->setNodeTransformation(transform);
    _rotateNode->attachSceneNode(meshNode);
    _angelMesh = std::make_shared<SVE::MeshEntity>("angel");
    _angelMesh->setMaterial("AngelMaterial");
    meshNode->attachEntity(_angelMesh);

    _debuffNode = engine->getSceneManager()->createSceneNode();
    std::shared_ptr<SVE::MeshEntity> debuffEntity = std::make_shared<SVE::MeshEntity>("cylinder");
    debuffEntity->setMaterial("DebuffMaterial");
    debuffEntity->setRenderLast();
    debuffEntity->setCastShadows(false);
    debuffEntity->getMaterialInfo()->diffuse = {1.0, 1.0, 1.0, 1.0 };
    _debuffNode->attachEntity(debuffEntity);

    _rootNode->attachSceneNode(addEnemyLightEffect(engine));
}

void Angel::update(float deltaTime)
{
    _ai->update(deltaTime);
    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, getHeight(), -realMapPos.x);
    auto transform = glm::translate(glm::mat4(1), position);
    _rootNode->setNodeTransformation(transform);
    const auto rotateAngle = 180.0f + 90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
    transform = glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0));
    _rotateNode->setNodeTransformation(transform);

    transform = glm::scale(glm::mat4(1), glm::vec3(1.0f, 2.0f, 1.0f));
    transform = glm::rotate(transform, SVE::Engine::getInstance()->getTime() * glm::radians(90.0f) * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    _debuffNode->setNodeTransformation(transform);
}

void Angel::increaseState(EnemyState state)
{
    Enemy::increaseState(state);
    switch (state)
    {
        case EnemyState::Frozen:
            break;
        case EnemyState::Vulnerable:
            _angelMesh->setMaterial("AngelBlinkMaterial");
            _rootNode->attachSceneNode(_debuffNode);
            break;
        case EnemyState::Dead:
            _rootNode->getParent()->detachSceneNode(_rootNode);
            break;
    }
}

void Angel::decreaseState(EnemyState state)
{
    Enemy::decreaseState(state);
    if (!isStateActive(state))
    {
        switch(state)
        {
            case EnemyState::Frozen:
                break;
            case EnemyState::Vulnerable:
                _angelMesh->setMaterial("AngelMaterial");
                _rootNode->detachSceneNode(_debuffNode);
                break;
            case EnemyState::Dead:
                _gameMap->mapNode->attachSceneNode(_rootNode);
                break;
        }
    }
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