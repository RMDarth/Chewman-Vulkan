// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include <Game/Enemies/RandomWalkerAI.h>
#include "SVE/SceneNode.h"
#include "MapTraveller.h"

union SDL_Event;

namespace Chewman
{

struct GameMap;

class Player
{
public:
    Player(GameMap* gameMap, glm::ivec2 startPos);

    void update(float deltaTime);
    void processInput(const SDL_Event& event);

private:
    void updateMovement(float deltaTime);
    bool checkForDeath();
    void playDeath();

private:
    // for debug
    bool _followMode = false;

    glm::ivec2 _startPos;
    std::shared_ptr<SVE::SceneNode> _rootNode;
    std::shared_ptr<SVE::SceneNode> _cameraAttachNode;
    std::shared_ptr<MapTraveller> _mapTraveller;
    GameMap* _gameMap;

    MoveDirection _nextMove;
};

} // namespace Chewman