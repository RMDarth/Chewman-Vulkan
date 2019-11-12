// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <SVE/ParticleSystemEntity.h>
#include "Bomb.h"

namespace Chewman
{

Bomb::Bomb(GameMap* gameMap, glm::ivec2 startPos)
    : PowerUp(gameMap, startPos, PowerUpType::Bomb)
    , _bombExplosionPS(std::make_shared<SVE::ParticleSystemEntity>("BombParticle"))
    , _bombSmokePS(std::make_shared<SVE::ParticleSystemEntity>("BombSmokeParticle"))
{
}

void Bomb::eat()
{
    _rootNode->attachEntity(_bombSmokePS);
    _rootNode->attachEntity(_bombExplosionPS);
    _rootNode->detachSceneNode(_rotateNode);
    _explosionTime = 1.5f;
}

void Bomb::update(float deltaTime)
{
    PowerUp::update(deltaTime);
    if (_explosionTime > 0)
    {
        _explosionTime -= deltaTime;
        if (_explosionTime < 1.0 && _emittersStopped == 0)
        {
            _emittersStopped++;
            _bombExplosionPS->getSettings().particleEmitter.emissionRate = 0;
        }
        if (_explosionTime < 0.5 && _emittersStopped == 1)
        {
            _emittersStopped++;
            _bombSmokePS->getSettings().particleEmitter.emissionRate = 0;
        }
        if (_explosionTime < 0)
        {
            PowerUp::eat();
        }
    }
}
} // namespace Chewman