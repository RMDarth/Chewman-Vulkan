// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <glm/glm.hpp>

namespace Chewman
{
class GameMapProcessor;

class GameRulesProcessor
{
public:
    explicit GameRulesProcessor(GameMapProcessor& gameMapProcessor);

    void update(float deltaTime);

private:
    void playDeath(float deltaTime);

private:
    GameMapProcessor& _gameMapProcessor;

    float _deathTime = 0;
    bool _deathSecondPhase = false;

    // Camera animation
    glm::vec3 _cameraStart[2]; // pos + angles
    glm::vec3 _cameraEnd[2];
};

} // namespace Chewman