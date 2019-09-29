// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "DefaultEnemy.h"

#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/MaterialManager.h"
#include "SVE/VulkanMaterial.h"
#include "SVE/MeshEntity.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Game/Level/GameMap.h"
#include "Game/Level/GameUtils.h"
#include "RandomWalkerAI.h"

namespace Chewman
{

DefaultEnemy::DefaultEnemy(GameMap* map, glm::ivec2 startPos, EnemyType enemyType,
                           const std::string& meshName, std::string normalMaterial,
                           int noReturnWayChance, float lightHeight)
        : Enemy(map, startPos, enemyType)
        , _normalMaterial(std::move(normalMaterial))
{
    _ai = std::make_shared<RandomWalkerAI>(*_mapTraveller, noReturnWayChance);

    auto* engine = SVE::Engine::getInstance();

    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    _rotateNode = engine->getSceneManager()->createSceneNode();
    _rootNode->attachSceneNode(_rotateNode);

    _rootNode->setNodeTransformation(transform);
    map->mapNode->attachSceneNode(_rootNode);

    _meshNode = engine->getSceneManager()->createSceneNode();
    _rotateNode->attachSceneNode(_meshNode);
    _enemyMesh = std::make_shared<SVE::MeshEntity>(meshName);
    _enemyMesh->setMaterial(_normalMaterial);
    _meshNode->attachEntity(_enemyMesh);

    _debuffNode = engine->getSceneManager()->createSceneNode();
    std::shared_ptr<SVE::MeshEntity> debuffEntity = std::make_shared<SVE::MeshEntity>("cylinder");
    debuffEntity->setMaterial("DebuffMaterial");
    debuffEntity->setRenderLast();
    debuffEntity->setCastShadows(false);
    debuffEntity->getMaterialInfo()->diffuse = {1.0, 1.0, 1.0, 1.0 };
    _debuffNode->attachEntity(debuffEntity);

    _rootNode->attachSceneNode(addEnemyLightEffect(engine, lightHeight));
}

void DefaultEnemy::update(float deltaTime)
{
    if (!isStateActive(EnemyState::Frozen))
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

void DefaultEnemy::increaseState(EnemyState state)
{
    Enemy::increaseState(state);
    switch (state)
    {
        case EnemyState::Frozen:
            _enemyMesh->setMaterial(isStateActive(EnemyState::Vulnerable) ? _frostVulnerableMaterial : _frostMaterial);
            _enemyMesh->resetTime();
            break;
        case EnemyState::Vulnerable:
            _enemyMesh->setMaterial(isStateActive(EnemyState::Frozen) ? _frostVulnerableMaterial : _vulnerableMaterial);
            _rootNode->attachSceneNode(_debuffNode);
            break;
        case EnemyState::Dead:
            _rootNode->getParent()->detachSceneNode(_rootNode);
            break;
    }
}

void DefaultEnemy::decreaseState(EnemyState state)
{
    Enemy::decreaseState(state);
    if (!isStateActive(state))
    {
        switch(state)
        {
            case EnemyState::Frozen:
                _enemyMesh->setMaterial(isStateActive(EnemyState::Vulnerable) ? _vulnerableMaterial : _normalMaterial);
                break;
            case EnemyState::Vulnerable:
                _enemyMesh->setMaterial(isStateActive(EnemyState::Frozen) ? _frostMaterial : _normalMaterial);
                _rootNode->detachSceneNode(_debuffNode);
                break;
            case EnemyState::Dead:
                _gameMap->mapNode->attachSceneNode(_rootNode);
                break;
        }
    }
}

float DefaultEnemy::getHeight()
{
    return 0.0f;
}

void DefaultEnemy::createMaterials()
{
    auto* materialManager = SVE::Engine::getInstance()->getMaterialManager();
    auto* baseMaterial = materialManager->getMaterial(_normalMaterial);
    auto baseMaterialSettings = baseMaterial->getVulkanMaterial()->getSettings();
    _vulnerableMaterial = "Blink" + _normalMaterial;
    _frostMaterial = "Frost" + _normalMaterial;
    _frostVulnerableMaterial = "Frost" + _vulnerableMaterial;

    auto setFragShader = [&](const std::string& materialName, const std::string& shaderName)
    {
        auto* material = materialManager->getMaterial(materialName, true);
        if (!material)
        {
            baseMaterialSettings.name = materialName;
            baseMaterialSettings.fragmentShaderName = shaderName;
            materialManager->registerMaterial(std::make_shared<SVE::Material>(baseMaterialSettings));
        }
    };

    setFragShader(_vulnerableMaterial, "phongShadowBlinkFragmentShader");
    setFragShader(_frostMaterial, "phongShadowFrostFragmentShader");
    setFragShader(_frostVulnerableMaterial, "phongShadowFrostBlinkFragmentShader");
}

} // namespace Chewman