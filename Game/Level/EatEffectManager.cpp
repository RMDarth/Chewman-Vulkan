// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "EatEffectManager.h"
#include "GameMap.h"
#include "GameUtils.h"
#include "Game/GraphicsSettings.h"
#include "Game/Game.h"
#include "BombFireEntity.h"
#include "CustomEntity.h"
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

    _isParticlesEnabled = Game::getInstance()->getGraphicsManager().getSettings().particleEffects != ParticlesSettings::None;

    auto getNodeWithPS = [&](const std::string& psName)
    {
        auto node = sceneManager->createSceneNode();
        if (_isParticlesEnabled)
            node->attachEntity(std::make_shared<SVE::ParticleSystemEntity>(psName));
        else
        {
            // TODO: Make this configurable
            BombFireInfo info {};
            info.radius = 1.5f;
            info.maxParticles = 600;
            info.percent = 0.0f;
            info.halfSize = 0.6f;
            node->attachEntity(std::make_shared<BombFireEntity>("SmokeMeshParticleMaterial", info));
        }
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
            if (_isParticlesEnabled)
            {
                auto* ps = static_cast<SVE::ParticleSystemEntity*>(effect.effectNode->getAttachedEntities().front().get());
                ps->getSettings().particleEmitter.emissionRate = 300.0f * std::max(0.0f, effect.time - 0.5f);
            } else {
                auto* entity = static_cast<BombFireEntity*>(effect.effectNode->getAttachedEntities().front().get());
                entity->getInfo().percent = 2.0f - effect.time;
                entity->getInfo().alpha = effect.time;
            }
        }
    }

    if (needRemove)
        _currentEffects.remove_if([](EffectInfo& info) { return info.time < 0; });
}

} // namespace Chewman