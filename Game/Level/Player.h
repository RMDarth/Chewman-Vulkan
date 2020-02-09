// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include "Game/Level/Enemies/RandomWalkerAI.h"
#include "SVE/SceneNode.h"
#include "MapTraveller.h"

union SDL_Event;

namespace SVE
{
class MeshEntity;
class ParticleSystemEntity;
}

namespace Chewman
{

struct GameMap;

class Player
{
public:
    Player(GameMap* gameMap, glm::ivec2 startPos);

    void update(float deltaTime);
    void processInput(const SDL_Event& event);

    std::shared_ptr<MapTraveller> getMapTraveller();
    void setCameraFollow(bool value);
    void resetPosition();
    void resetPlaying();
    void playDeathAnimation();
    void playPowerUpAnimation();
    void playPowerDownAnimation();

    void setIsDying(bool isDying);
    bool isDying();

    void enableLight(bool enable);

    void setNextMove(MoveDirection direction);
    MoveDirection getNextMove() const;

private:
    void updateMovement(float deltaTime);
    void createAppearEffect();
    void createDisappearEffect();
    void createPowerUpEffect();
    void showAppearEffect(bool show);
    void showDisappearEffect(bool show);
    void updateAppearEffect();

private:
    // for debug
    bool _followMode = true;

    bool _isCameraFollow = true;
    int _startSlideX = 0;
    int _startSlideY = 0;
    bool _isSliding = false;

    float _appearTime = 0.0f;
    bool _appearing = false;

    glm::ivec2 _startPos;
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _rotateNode;
    std::shared_ptr<SVE::SceneNode> _lightNode;
    std::shared_ptr<SVE::MeshEntity> _trashmanEntity;
    std::shared_ptr<SVE::MeshEntity> _powerUpEntity;
    std::shared_ptr<SVE::ParticleSystemEntity> _powerUpPS;

    std::shared_ptr<MapTraveller> _mapTraveller;
    GameMap* _gameMap;

    std::shared_ptr<SVE::SceneNode> _appearNode;
    std::shared_ptr<SVE::SceneNode> _appearNodeGlow;
    std::shared_ptr<SVE::SceneNode> _disappearNode;

    std::shared_ptr<SVE::SceneNode> _powerUpEffectNode;
    float _powerUpTime = 0.0f;
    bool _isDying = false;

    MoveDirection _nextMove = MoveDirection::None;

    glm::vec3 _accelBasis = {};
    bool _basisInitialized = false;
};

} // namespace Chewman