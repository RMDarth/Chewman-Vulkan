// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <glm/glm.hpp>
#include "PowerUp.h"

namespace Chewman
{
class GameMapProcessor;
class Player;

struct GameAffector
{
    PowerUpType powerUp;
    float remainingTime;
};


class GameRulesProcessor
{
public:
    explicit GameRulesProcessor(GameMapProcessor& gameMapProcessor);

    void update(float deltaTime);

private:
    void playDeath(float deltaTime);

    void updateAffectors(float deltaTime);
    void activatePowerUp(PowerUpType type);
    void deactivatePowerUp(PowerUpType type);

    std::shared_ptr<Player>& getPlayer();

private:
    GameMapProcessor& _gameMapProcessor;
    std::shared_ptr<Player> _player;

    // change to map
    std::list<GameAffector> _gameAffectors;
    uint8_t _activeState[PowerUpCount] = {};

    float _deathTime = 0;
    bool _deathSecondPhase = false;

    // Camera animation
    glm::vec3 _cameraStart[2]; // pos + angles
    glm::vec3 _cameraEnd[2];
};

} // namespace Chewman