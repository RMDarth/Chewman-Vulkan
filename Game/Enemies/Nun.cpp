// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Nun.h"

#include <SVE/Engine.h>
#include <SVE/SceneManager.h>
#include <SVE/MeshEntity.h>
#include <SVE/LightManager.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Game/GameMap.h"
#include "RandomWalkerAI.h"

namespace Chewman
{
namespace
{

std::shared_ptr<SVE::LightNode> addLightEffect(SVE::Engine* engine)
{
    SVE::LightSettings lightSettings {};
    lightSettings.lightType = SVE::LightType::PointLight;
    lightSettings.castShadows = false;
    lightSettings.diffuseStrength = glm::vec4(1.0);
    lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    lightSettings.ambientStrength = { 0.2f, 0.2f, 0.2f, 1.0f };
    lightSettings.shininess = 16;
    lightSettings.constAtten = 1.0f * 1.8f;
    lightSettings.linearAtten = 0.35f * 0.25f;
    lightSettings.quadAtten = 0.44f * 0.25f;
    auto lightManager = engine->getSceneManager()->getLightManager();
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
    lightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 1.5, 0)));
    lightManager->setLight(lightNode, lightManager->getLightCount());

    return lightNode;
}

} // anon namespace

Nun::Nun(GameMap* map, glm::ivec2 startPos)
    : Enemy(map, startPos)
{
    _ai = std::make_shared<RandomWalkerAI>(*_mapTraveller, 85);

    auto* engine = SVE::Engine::getInstance();

    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);

    _rootNode->setNodeTransformation(transform);
    map->mapNode->attachSceneNode(_rootNode);

    auto nunMesh = std::make_shared<SVE::MeshEntity>("nun");
    nunMesh->setMaterial("NunMaterial");
    _rootNode->attachEntity(nunMesh);

    _rootNode->attachSceneNode(addLightEffect(engine));
}


void Nun::update()
{
    _ai->update();
    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);
    auto transform = glm::translate(glm::mat4(1), position);
    const auto rotateAngle = 180.0f + 90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
    transform = glm::rotate(transform, glm::radians(rotateAngle), glm::vec3(0, 1, 0));
    _rootNode->setNodeTransformation(transform);
}

} // namespace Chewman