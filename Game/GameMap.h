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
#include "GameRulesProcessor.h"

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

    uint32_t activeCoins;

    CellInfoMap mapData;
    size_t width;
    size_t height;
};

enum class GameMapState
{
    Game,
    Pause,
    Animation
};

class GameMapProcessor
{
public:
    explicit GameMapProcessor(std::shared_ptr<GameMap> gameMap);
    ~GameMapProcessor();

    void update(float time);
    void hide();
    void processInput(const SDL_Event& event);
    void setState(GameMapState gameState);
    GameMapState getState() const;
    std::shared_ptr<GameMap> getGameMap();

    float getDeltaTime();

private:
    void updateGargoyle(float time, Gargoyle& gargoyle);
    void updateTeleport(float time, Teleport& teleport);
    void updateCoin(float time, Coin& coin);

private:
    GameRulesProcessor _gameRulesProcessor;
    GameMapState _state = GameMapState::Game;
    std::shared_ptr<GameMap> _gameMap;
    float _totalTime;
    float _deltaTime = 0;
};

} // namespace Chewman