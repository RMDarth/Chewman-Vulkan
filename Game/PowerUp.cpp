// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "PowerUp.h"
#include "GameMap.h"
#include "GameUtils.h"
#include "SVE/SceneManager.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{
namespace
{
PowerUpType getPowerUpType(char symbolType)
{
    switch (symbolType)
    {
        case 'P':
            return PowerUpType::Pentagram;
        case 'F':
            return PowerUpType::Freeze;
        case 'A':
            return PowerUpType::Acceleration;
        case 'X':
            return PowerUpType::Life;
        case 'B':
            return PowerUpType::Bomb;
        case 'H':
            return PowerUpType::Jackhammer;
        case 'T':
            return PowerUpType::Teeth;
    }

    assert(!"Unknown power-up");
    return PowerUpType::Pentagram;
}

std::shared_ptr<SVE::MeshEntity> getPowerUpEntity(PowerUpType type)
{
    std::string meshName;
    std::string materialName;

    switch (type)
    {
        case PowerUpType::Pentagram:
            meshName = "pentagram";
            materialName = "PentagramMaterial";
            break;
        case PowerUpType::Freeze:
            meshName = "freeze";
            materialName = "FreezeMaterial";
            break;
        case PowerUpType::Acceleration:
            meshName = "acceleration";
            materialName = "AccelerationMaterial";
            break;
        case PowerUpType::Life:
            meshName = "life";
            materialName = "LifeMaterial";
            break;
        case PowerUpType::Bomb:
            meshName = "bomb";
            materialName = "BombMaterial";
            break;
        case PowerUpType::Jackhammer:
            meshName = "jackhammer";
            materialName = "JackhammerMaterial";
            break;
        case PowerUpType::Teeth:
            meshName = "teeth";
            materialName = "TeethMaterial";
            break;
    }

    auto powerUpMesh = std::make_shared<SVE::MeshEntity>(meshName);
    powerUpMesh->setMaterial(materialName);

    return powerUpMesh;
}

} // anon namespace

PowerUp::PowerUp(GameMap* gameMap, glm::ivec2 startPos, char symbolType)
    : PowerUp(gameMap, startPos, getPowerUpType(symbolType))
{
}

PowerUp::PowerUp(GameMap* gameMap, glm::ivec2 startPos, PowerUpType type)
    : _type(type)
{
    auto* engine = SVE::Engine::getInstance();

    auto position = getWorldPos(startPos.x, startPos.y, 1.0f);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    switch (_type)
    {
        case PowerUpType::Pentagram:
        case PowerUpType::Freeze:
        case PowerUpType::Acceleration:
        case PowerUpType::Teeth:
            transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case PowerUpType::Life:
            transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case PowerUpType::Bomb:
        case PowerUpType::Jackhammer:
            break;
    }


    _rootNode->setNodeTransformation(transform);
    gameMap->mapNode->attachSceneNode(_rootNode);

    auto powerUpEntity = getPowerUpEntity(type);
    _rootNode->attachEntity(std::move(powerUpEntity));

    rotateItem(std::uniform_real_distribution<float>(0.0f, 5.0f)(getRandomEngine()));
}

void PowerUp::rotateItem(float time)
{
    auto transform = _rootNode->getNodeTransformation();
    switch (_type)
    {
        case PowerUpType::Pentagram:
        case PowerUpType::Freeze:
            transform = glm::rotate(transform, time, glm::vec3(1.0f, 1.0f, 1.0f));
            break;
        case PowerUpType::Acceleration:
        case PowerUpType::Life:
        case PowerUpType::Teeth:
            transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f));
            break;

        case PowerUpType::Bomb:
        case PowerUpType::Jackhammer:
            transform = glm::rotate(transform, time, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
    }

    _rootNode->setNodeTransformation(transform);
}

void PowerUp::update(float deltaTime)
{
    rotateItem(deltaTime);
}

PowerUpType PowerUp::getType() const
{
    return _type;
}

} // namespace Chewman