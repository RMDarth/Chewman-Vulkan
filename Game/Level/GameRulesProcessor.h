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
class Enemy;
class MapTraveller;

constexpr float JackhammerTotalTime = 10.0f;
constexpr float FreezeTotalTime = 16.0f;
constexpr float PentagrammTotalTime = 16.0f;
constexpr float AccelerationTotalTime = 13.0f;
constexpr float TeethTotalTime = 10.0f;
constexpr float SlowTotalTime = 10.0f;

struct GameAffector
{
    PowerUpType powerUp;
    float remainingTime;
    bool isPoweredDown = false;
};

class GameRulesProcessor
{
public:
    explicit GameRulesProcessor(GameMapProcessor& gameMapProcessor);

    void runStartLevelAnimation();
    void update(float deltaTime);

    std::map<PowerUpType, float> getCurrentAffectors() const;

private:
    void playDeath(float deltaTime);
    void resetLevel();

    void updateAffectors(float deltaTime);
    void activatePowerUp(PowerUpType type, glm::ivec2 pos, Enemy* eater = nullptr);
    void deactivatePowerUp(PowerUpType type);

    template<typename Func>
    bool isGargoyleAffecting(glm::vec3 realPos, const std::shared_ptr<MapTraveller>& mapTraveller, Func func);

    std::shared_ptr<Player>& getPlayer();

    void updateCameraAnimation(float deltaTime);
    void destroyWalls(glm::ivec2 pos);
    void updateWallsDown(float deltaTime);
    void regenerateMap();
    bool eatCoin(glm::ivec2 pos);
    void setShadowCamera(bool isZoomed);

private:
    GameMapProcessor& _gameMapProcessor;
    std::shared_ptr<Player> _player;

    // change to map
    std::list<GameAffector> _gameAffectors;
    uint8_t _activeState[PowerUpCount] = {};
    PowerUpType _lastSpeedPowerUp = {};

    float _deathTime = 0;
    bool _deathSecondPhase = false;

    float _wallsDownTime = 0;

    bool _insideTeleport = false;

    // Camera animation
    bool _isCameraMoving = false;
    float _cameraTime = 0.0f;
    float _cameraSpeed = 1.0f;
    glm::vec3 _cameraStart[2] = {}; // pos + angles
    glm::vec3 _cameraEnd[2] = {};
};

} // namespace Chewman