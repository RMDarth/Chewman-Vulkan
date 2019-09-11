// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "StaticObject.h"
#include "GameMap.h"
#include "GameUtils.h"
#include "SVE/SceneManager.h"

#include <glm/gtc/matrix_transform.hpp>

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

    switch (type)
    {
        case StaticObjectType::Tomb:
            meshName = "tomb";
            materialName = "TombMaterial";
            break;
        case StaticObjectType::Volcano:
            meshName = "tomb";
            materialName = "TombMaterial";
            break;
        case StaticObjectType::Dragon:
            meshName = "tomb";
            materialName = "TombMaterial";
            break;
        case StaticObjectType::Pot:
            meshName = "tomb";
            materialName = "TombMaterial";
            break;
        case StaticObjectType::Mouth:
            meshName = "tomb";
            materialName = "TombMaterial";
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
            break;
        case StaticObjectType::Dragon:
            break;
        case StaticObjectType::Pot:
            break;
        case StaticObjectType::Mouth:
            break;
    }

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    switch (_type)
    {
        case StaticObjectType::Tomb:
            break;
        case StaticObjectType::Volcano:
        case StaticObjectType::Dragon:
        case StaticObjectType::Pot:
        case StaticObjectType::Mouth:
            //transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
    }
    transform = glm::rotate(transform, glm::radians(90.0f * (rotation - '1')), glm::vec3(0.0f, 1.0f, 0.0f));

    _rootNode->setNodeTransformation(transform);
    gameMap->mapNode->attachSceneNode(_rootNode);

    auto objectEntity = getStaticObjectEntity(type);
    _rootNode->attachEntity(std::move(objectEntity));
}

StaticObjectType StaticObject::getType() const
{
    return _type;
}

void StaticObject::update()
{
    // update animated models?
}

} // namespace Chewman