// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "PowerUp.h"

namespace Chewman
{

class Bomb : public PowerUp
{
public:
    Bomb(GameMap* gameMap, glm::ivec2 startPos);

    void eat() override;
    void update(float deltaTime) override;

private:
    float _explosionTime = -1;
    uint32_t _emittersStopped = 0;
    std::shared_ptr<SVE::ParticleSystemEntity> _bombExplosionPS;
    std::shared_ptr<SVE::ParticleSystemEntity> _bombSmokePS;
};

} // namespace Chewman