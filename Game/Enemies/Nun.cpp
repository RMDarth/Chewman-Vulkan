// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
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
    _rotateNode = engine->getSceneManager()->createSceneNode();
    _rootNode->attachSceneNode(_rotateNode);

    _rootNode->setNodeTransformation(transform);
    map->mapNode->attachSceneNode(_rootNode);

    _nunMesh = std::make_shared<SVE::MeshEntity>("nun");
    _nunMesh->setMaterial("NunMaterial");
    _rotateNode->attachEntity(_nunMesh);


    _debuffNode = engine->getSceneManager()->createSceneNode();
    //_rootNode->attachSceneNode(_debuffNode);
    std::shared_ptr<SVE::MeshEntity> debuffEntity = std::make_shared<SVE::MeshEntity>("cylinder");
    debuffEntity->setMaterial("DebuffMaterial");
    debuffEntity->setRenderLast();
    debuffEntity->setCastShadows(false);
    debuffEntity->getMaterialInfo()->diffuse = {1.0, 1.0, 1.0, 1.0 };
    _debuffNode->attachEntity(debuffEntity);

    _rootNode->attachSceneNode(addLightEffect(engine));
}


void Nun::update(float deltaTime)
{
    _ai->update(deltaTime);
    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);
    auto transform = glm::translate(glm::mat4(1), position);
    _rootNode->setNodeTransformation(transform);
    const auto rotateAngle = 180.0f + 90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
    transform = glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0));
    _rotateNode->setNodeTransformation(transform);

    transform = glm::scale(glm::mat4(1), glm::vec3(1.0f, 2.0f, 1.0f));
    transform = glm::rotate(transform, SVE::Engine::getInstance()->getTime() * glm::radians(90.0f) * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    _debuffNode->setNodeTransformation(transform);
}


void Nun::increaseState(EnemyState state)
{
    Enemy::increaseState(state);
    switch (state)
    {
        case EnemyState::Frozen:
            break;
        case EnemyState::Vulnerable:
            _nunMesh->setMaterial("NunBlinkMaterial");
            _rootNode->attachSceneNode(_debuffNode);
            break;
        case EnemyState::Dead:
            _rootNode->getParent()->detachSceneNode(_rootNode);
            break;
    }
}

void Nun::decreaseState(EnemyState state)
{
    Enemy::decreaseState(state);
    if (!isStateActive(state))
    {
        switch(state)
        {
            case EnemyState::Frozen:
                break;
            case EnemyState::Vulnerable:
                _nunMesh->setMaterial("NunMaterial");
                _rootNode->detachSceneNode(_debuffNode);
                break;
            case EnemyState::Dead:
                _gameMap->mapNode->attachSceneNode(_rootNode);
                break;
        }
    }
}

} // namespace Chewman