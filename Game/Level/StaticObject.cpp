// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "StaticObject.h"
#include "GameMap.h"
#include "GameUtils.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <Game/Game.h>

namespace Chewman
{
namespace
{

StaticObjectType getObjectType(char symbolType)
{
    switch (symbolType)
    {
        case 'J':
            return StaticObjectType::Tomb;
        case 'V':
            return StaticObjectType::Volcano;
        case 'D':
            return StaticObjectType::Dragon;
        case 'Z':
            return StaticObjectType::Pot;
        case 'Y':
            return StaticObjectType::Mouth;
    }

    assert(!"Unknown static object");
    return StaticObjectType::Tomb;
}

std::shared_ptr<SVE::MeshEntity> getStaticObjectEntity(StaticObjectType type)
{
    std::string meshName;
    std::string materialName;

    const auto& graphicSettings = Game::getInstance()->getGraphicsManager().getSettings();

    switch (type)
    {
        case StaticObjectType::Tomb:
            meshName = "tomb";
            materialName = "TombMaterial";
            break;
        case StaticObjectType::Volcano:
            meshName = "volcano";
            materialName = graphicSettings.effectSettings != EffectSettings::Low ? "VolcanoMaterial" : "VolcanoLowMaterial";
            break;
        case StaticObjectType::Dragon:
            meshName = "dragon";
            materialName = "DragonMaterial";
            break;
        case StaticObjectType::Pot:
            meshName = "pot";
            materialName = graphicSettings.effectSettings != EffectSettings::Low ? "PotMaterial" : "PotLowMaterial";
            break;
        case StaticObjectType::Mouth:
            meshName = "throat";
            materialName = "ThroatMaterial";
            break;
    }

    auto staticMeshEntity = std::make_shared<SVE::MeshEntity>(meshName);
    staticMeshEntity->setMaterial(materialName);

    return staticMeshEntity;
}

} // anon namespace

StaticObject::StaticObject(GameMap* gameMap, glm::ivec2 startPos, char symbolType, char rotation)
    : StaticObject(gameMap, startPos, getObjectType(symbolType), rotation)
{
}

StaticObject::StaticObject(GameMap* gameMap, glm::ivec2 startPos, StaticObjectType type, char rotation)
    : _type(type)
{
    auto* engine = SVE::Engine::getInstance();

    bool isHorizontal = rotation == '2' || rotation == '4';
    auto position = getWorldPos(startPos.x, startPos.y, 0.0f);

    switch (_type)
    {
        case StaticObjectType::Tomb:
            position -= isHorizontal ? glm::vec3(CellSize * 0.5f, 0.0, -CellSize * 1.5f)
                                     : glm::vec3(CellSize * -0.5f, 0.0, -CellSize * 0.5f);
            break;
        case StaticObjectType::Volcano:
            position += glm::vec3(CellSize * 0.5f, 0.0, CellSize * 1.5f);
            break;
        case StaticObjectType::Dragon:
            position += glm::vec3(CellSize * 0.0f, 0.0, CellSize * 1.0f);
            break;
        case StaticObjectType::Pot:
            position += glm::vec3(CellSize * 0.0f, 0.0, CellSize * 1.0f);
            break;
        case StaticObjectType::Mouth:
            position -= isHorizontal ? glm::vec3(CellSize * 0.5f, 0.0, -CellSize * 1.5f)
                                     : glm::vec3(CellSize * -0.5f, 0.0, -CellSize * 0.5f);
            break;
    }

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    switch (_type)
    {
        case StaticObjectType::Tomb:
            transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            transform = glm::rotate(transform, glm::radians(90.0f * (rotation - '1')), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case StaticObjectType::Volcano:
            transform = glm::rotate(transform, glm::radians(-90.0f * (rotation - '1')), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case StaticObjectType::Mouth:
            transform = glm::rotate(transform, glm::radians(-90.0f * (rotation - '0')), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case StaticObjectType::Dragon:
            transform = glm::translate(transform, glm::vec3(0.0f, 1.5f, 0.0f));
            transform = glm::rotate(transform, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            transform = glm::rotate(transform, glm::radians(-90.0f * (rotation - '1')), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case StaticObjectType::Pot:
            transform = glm::translate(transform, glm::vec3(0.0f, 1.3f, 0.0f));
            transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            transform = glm::rotate(transform, glm::radians(-90.0f * (rotation - '1')), glm::vec3(0.0f, 0.0f, 1.0f));
            break;
    }


    _rootNode->setNodeTransformation(transform);
    gameMap->mapNode->attachSceneNode(_rootNode);

    auto objectEntity = getStaticObjectEntity(type);
    _rootNode->attachEntity(std::move(objectEntity));
}

StaticObjectType StaticObject::getType() const
{
    return _type;
}

void StaticObject::update(float deltaTime)
{
    // update animated models?
}

CellType StaticObject::getCellType(char type)
{
    switch (getObjectType(type))
    {
        case StaticObjectType::Tomb:
            return CellType::InvisibleWallWithFloor;
        case StaticObjectType::Volcano:
            return CellType::InvisibleWallWithFloor;
        case StaticObjectType::Dragon:
            return CellType::InvisibleWallWithFloor;
        case StaticObjectType::Pot:
            return CellType::Liquid;
        case StaticObjectType::Mouth:
            return CellType::InvisibleWallEmpty;
    }

    return CellType::InvisibleWallWithFloor;
}

std::pair<size_t, size_t> StaticObject::getSize(char type, char rotation)
{
    switch (getObjectType(type))
    {
        case StaticObjectType::Tomb:
            if (rotation == '1' || rotation == '3')
                return {2, 4};
            return {4, 2};
        case StaticObjectType::Volcano:
            return { 4, 4 };
        case StaticObjectType::Dragon:
            return { 3, 3 };
        case StaticObjectType::Pot:
            return { 3, 3 };
        case StaticObjectType::Mouth:
            if (rotation == '1' || rotation == '3')
                return {2, 4};
            return {4, 2};
    }
    assert(!"Incorrect type");
    return {0, 0};
}

} // namespace Chewman