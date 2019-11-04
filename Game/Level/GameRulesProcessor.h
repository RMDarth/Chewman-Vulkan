// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <glm/glm.hpp>
#include <future>
#include "PowerUp.h"

namespace SVE
{
class MeshEntity;
class Mesh;
}

namespace Chewman
{
class GameMapProcessor;
class Player;

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

    void update(float deltaTime);

private:
    void playDeath(float deltaTime);

    void updateAffectors(float deltaTime);
    void activatePowerUp(PowerUpType type, glm::ivec2 pos);
    void deactivatePowerUp(PowerUpType type);

    std::shared_ptr<Player>& getPlayer();

    void updateCameraAnimation(float deltaTime);
    void destroyWalls(glm::ivec2 pos);
    void updateWallsDown(float deltaTime);
    void regenerateMap();
    void updateRegeneration(bool forceFinish);

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

    // async map changes
    std::future<void> _mapChangesFuture;
    std::array<std::shared_ptr<SVE::Mesh>, 3> _preparedMeshes;
    std::list<std::shared_ptr<SVE::Mesh>> _oldMeshes;
    std::list<std::shared_ptr<SVE::Entity>> _oldEntities;
    std::atomic_bool _regenerationFinished;
    bool _useSuffix = true;
};

} // namespace Chewman