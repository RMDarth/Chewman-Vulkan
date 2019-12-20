// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"

namespace Chewman
{

class ChewmanEnemy : public DefaultEnemy
{
public:
    ChewmanEnemy(GameMap* map, glm::ivec2 startPos);

    void resetAll() override;

    void update(float deltaTime) override;

    void increaseState(EnemyState state) override;
    void decreaseState(EnemyState state) override;

private:
    float _deathAnimation = 0;
    bool _isDeathAnimation = false;
};

} // namespace Chewman