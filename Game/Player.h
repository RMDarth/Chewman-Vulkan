// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include <Game/Enemies/RandomWalkerAI.h>
#include "SVE/SceneNode.h"
#include "MapTraveller.h"

union SDL_Event;

namespace SVE
{
class MeshEntity;
}

namespace Chewman
{

struct GameMap;

struct PlayerInfo
{
    uint32_t lives = 2;
    bool isDying = false;
    uint32_t points = 0;
};

class Player
{
public:
    Player(GameMap* gameMap, glm::ivec2 startPos);

    void update(float deltaTime);
    void processInput(const SDL_Event& event);

    std::shared_ptr<MapTraveller> getMapTraveller();
    PlayerInfo* getPlayerInfo();
    void setCameraFollow(bool value);
    void resetPosition();
    void resetPlaying();
    void playDeathAnimation();
    void playPowerUpAnimation();

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
    bool _followMode = false;

    bool _isCameraFollow = true;

    float _appearTime = 0.0f;
    bool _appearing = false;

    glm::ivec2 _startPos;
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _rotateNode;
    std::shared_ptr<SVE::MeshEntity> _trashmanEntity;
    std::shared_ptr<SVE::MeshEntity> _powerUpEntity;
    std::shared_ptr<MapTraveller> _mapTraveller;
    std::unique_ptr<PlayerInfo> _playerInfo;
    GameMap* _gameMap;

    std::shared_ptr<SVE::SceneNode> _appearNode;
    std::shared_ptr<SVE::SceneNode> _appearNodeGlow;
    std::shared_ptr<SVE::SceneNode> _disappearNode;

    std::shared_ptr<SVE::SceneNode> _powerUpEffectNode;
    float _powerUpTime = 0.0f;

    MoveDirection _nextMove;
};

} // namespace Chewman