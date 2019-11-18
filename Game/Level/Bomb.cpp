// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <SVE/ParticleSystemEntity.h>
#include "Bomb.h"
#include "CustomEntity.h"
#include "Game/Game.h"

namespace Chewman
{

Bomb::Bomb(GameMap* gameMap, glm::ivec2 startPos)
    : PowerUp(gameMap, startPos, PowerUpType::Bomb)
{
    if (Game::getInstance()->getGraphicsManager().getSettings().particleEffects == ParticlesSettings::None)
    {
        BombFireInfo info {};
        _bombFire = std::make_shared<BombFireEntity>("BombFireMaterial", info);
        _isParticles = false;
    } else {
        _bombExplosionPS = std::make_shared<SVE::ParticleSystemEntity>("BombParticle");
        _bombSmokePS = std::make_shared<SVE::ParticleSystemEntity>("BombSmokeParticle");
        _isParticles = true;
    }
}

void Bomb::eat()
{
    if (_isParticles)
    {
        _rootNode->attachEntity(_bombSmokePS);
        _rootNode->attachEntity(_bombExplosionPS);
    } else {
        _rootNode->attachEntity(_bombFire);
    }
    _rootNode->detachSceneNode(_rotateNode);
    _explosionTime = 1.5f;
}

void Bomb::update(float deltaTime)
{
    PowerUp::update(deltaTime);
    if (_explosionTime > 0)
    {
        _explosionTime -= deltaTime;
        if (!_isParticles)
            _bombFire->getInfo().percent = 1.5f - _explosionTime;
        if (_explosionTime < 1.0 && _emittersStopped == 0)
        {
            _emittersStopped++;
            if (_isParticles)
                _bombExplosionPS->getSettings().particleEmitter.emissionRate = 0;
        }
        if (_explosionTime < 0.5 && _emittersStopped == 1)
        {
            _emittersStopped++;
            if (_isParticles)
                _bombSmokePS->getSettings().particleEmitter.emissionRate = 0;
            else
                _rootNode->detachEntity(_bombFire);
        }
        if (_explosionTime < 0)
        {
            PowerUp::eat();
        }
    }
}
} // namespace Chewman