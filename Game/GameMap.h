// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include <memory>
#include <SVE/SceneNode.h>
#include <SVE/MeshEntity.h>
#include <SVE/Mesh.h>
#include "GameDefs.h"
#include "BlockMeshGenerator.h"
#include "Gargoyle.h"
#include "Teleport.h"
#include "Coin.h"
#include "PowerUp.h"
#include "Player.h"
#include "StaticObject.h"
#include "Enemies/Nun.h"

namespace Chewman
{

struct GameMap
{
    std::shared_ptr<SVE::SceneNode> mapNode;
    std::shared_ptr<SVE::MeshEntity> mapEntity[3];
    std::shared_ptr<SVE::MeshEntity> smokeEntity;
    std::shared_ptr<SVE::MeshEntity> lavaEntity;

    std::shared_ptr<Player> player;
    std::vector<Gargoyle> gargoyles;
    std::vector<Teleport> teleports;
    std::vector<Coin> coins;
    std::vector<PowerUp> powerUps;
    std::vector<StaticObject> staticObjects;
    std::vector<Nun> nuns;

    CellInfoMap mapData;
    size_t width;
    size_t height;
};

enum class GameState
{
    Game,
    Pause,
    Animation
};

class GameMapProcessor
{
public:
    explicit GameMapProcessor(std::shared_ptr<GameMap> gameMap);

    void update(float time);
    void processInput(const SDL_Event& event);

private:
    void updateGargoyle(float time, Gargoyle& gargoyle);
    void updateTeleport(float time, Teleport& teleport);
    void updateCoin(float time, Coin& coin);

private:
    std::shared_ptr<GameMap> _gameMap;
    float _totalTime;
};

} // namespace Chewman