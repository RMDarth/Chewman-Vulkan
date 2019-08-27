// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameMap.h"
#include "SVE/Engine.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{
namespace
{

void updateGargoyleParticles(Gargoyle& gargoyle, float life, float alphaChanger, float lifeDrain)
{
    auto& emitter = gargoyle.particleSystem->getSettings().particleEmitter;
    auto& affector = gargoyle.particleSystem->getSettings().particleAffector;
    emitter.minLife = life;
    emitter.maxLife = life;
    affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, alphaChanger);
    affector.lifeDrain = lifeDrain;
}

} // anon namespace

GameMapProcessor::GameMapProcessor(std::shared_ptr<GameMap> gameMap)
    : _gameMap(std::move(gameMap))
{
}

void GameMapProcessor::update(float time)
{
    if (time <= 0.0)
        return;

    for (auto& teleport : _gameMap->teleports)
    {
        updateTeleport(time, teleport);
    }

    for (auto& coin : _gameMap->coins)
    {
        updateCoin(time, coin);
    }

    for (auto& gargoyle : _gameMap->gargoyles)
    {
        updateGargoyle(time, gargoyle);
    }

    for (auto& nun : _gameMap->nuns)
    {
        nun.update();
    }
}

void GameMapProcessor::updateGargoyle(float time, Gargoyle& gargoyle)
{
    gargoyle.currentTime += time;
    switch (gargoyle.state)
    {
        case GargoyleState::Fire:
        {
            if (gargoyle.currentTime > gargoyle.fireTime - 0.2f)
            {
                gargoyle.state = GargoyleState::Rest;
                gargoyle.currentTime = 0;
                gargoyle.currentLength = 0;
                updateGargoyleParticles(gargoyle, 0.0f, -3.5f, 5.0f);
            } else {
                float finishPercent = gargoyle.currentTime / gargoyle.fireTime;
                gargoyle.currentLength = (float)gargoyle.lengthInCells * finishPercent;

                gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::LineLight;
                gargoyle.lightNode->getLightSettings().secondPoint = gargoyle.startPoint + gargoyle.direction * gargoyle.totalLength * finishPercent;
            }
            break;
        }
        case GargoyleState::Rest:
        {
            if (gargoyle.currentTime > 0.2f)
            {
                gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::None;
                gargoyle.lightNode->getLightSettings().secondPoint = gargoyle.startPoint;
            }
            if (gargoyle.currentTime > gargoyle.restTime + 0.2f)
            {
                gargoyle.state = GargoyleState::Fire;
                gargoyle.currentTime = 0;
                gargoyle.currentLength = 0;
                updateGargoyleParticles(gargoyle, 10.0f, 0.0f, 0.0f);
            }
            break;
        }
    }
}

void GameMapProcessor::updateTeleport(float time, Teleport &teleport)
{
    float currentTime = SVE::Engine::getInstance()->getTime();
    auto updateNode = [](std::shared_ptr<SVE::SceneNode>& node, float time)
    {
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        node->setNodeTransformation(nodeTransform);
    };

    updateNode(teleport.circleNode, currentTime * 2);
    updateNode(teleport.glowNode, currentTime * 5);
}

void GameMapProcessor::updateCoin(float time, Coin &coin)
{
    auto transform = coin.rootNode->getNodeTransformation();
    transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f));
    coin.rootNode->setNodeTransformation(transform);
}

} // namespace Chewman