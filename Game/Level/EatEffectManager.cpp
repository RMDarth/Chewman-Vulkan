// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "EatEffectManager.h"
#include "GameMap.h"
#include "GameUtils.h"
#include <SVE/Engine.h>
#include <SVE/SceneManager.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

constexpr uint32_t EatEffectPoolSize = 5;

EatEffectManager::EatEffectManager(Chewman::GameMap *gameMap)
    : _gameMap(gameMap)
{
    auto* sceneManager = SVE::Engine::getInstance()->getSceneManager();

    auto getNodeWithPS = [&](const std::string& psName)
    {
        auto node = sceneManager->createSceneNode();
        node->attachEntity(std::make_shared<SVE::ParticleSystemEntity>(psName));
        return node;
    };

    for (auto i = 0; i < EatEffectPoolSize; ++i)
    {
        _effectsPool[0].push(getNodeWithPS("TeethSmokeParticle"));
        _effectsPool[1].push(getNodeWithPS("TeethSmokeParticle"));
    }
}

void EatEffectManager::addEffect(EatEffectType type, glm::ivec2 pos)
{
    auto index = static_cast<uint8_t>(type);

    auto node = _effectsPool[index].front();
    _effectsPool[index].pop();
    _effectsPool[index].push(node);

    node->setNodeTransformation(glm::translate(glm::mat4(1), getWorldPos(pos.x, pos.y)));
    _gameMap->mapNode->attachSceneNode(node);
    _currentEffects.push_back({1.0f, node});
}

void EatEffectManager::update(float deltaTime)
{
    bool needRemove = false;
    for (auto& effect : _currentEffects)
    {
        effect.time -= deltaTime;
        if (effect.time < 0)
        {
            _gameMap->mapNode->detachSceneNode(effect.effectNode);
            needRemove = true;
        } else {
            auto* ps = static_cast<SVE::ParticleSystemEntity*>(effect.effectNode->getAttachedEntities().front().get());
            ps->getSettings().particleEmitter.emissionRate = 300.0f * std::max(0.0f, effect.time - 0.5f);
        }
    }

    if (needRemove)
        _currentEffects.remove_if([](EffectInfo& info) { return info.time < 0; });
}

} // namespace Chewman