// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include <memory>
#include <SVE/SceneNode.h>
#include <SVE/MeshEntity.h>
#include <SVE/Mesh.h>
#include "GameMapDefs.h"
#include "BlockMeshGenerator.h"
#include "Gargoyle.h"
#include "Teleport.h"
#include "Coin.h"
#include "PowerUp.h"
#include "Player.h"
#include "StaticObject.h"
#include "EatEffectManager.h"
#include "Game/Level/Enemies/Enemy.h"
#include "GameRulesProcessor.h"

namespace Chewman
{

struct GameMap
{
    std::string name;
    std::shared_ptr<SVE::SceneNode> mapNode;
    std::shared_ptr<SVE::SceneNode> upperLevelMeshNode;
    std::shared_ptr<SVE::MeshEntity> mapEntity[3];
    std::shared_ptr<SVE::MeshEntity> smokeEntity;
    std::shared_ptr<SVE::MeshEntity> smokeNAEntity;
    std::shared_ptr<SVE::MeshEntity> lavaEntity;

    std::shared_ptr<Player> player;
    std::vector<Gargoyle> gargoyles;
    std::vector<Teleport> teleports;
    std::vector<Coin> coins;
    std::vector<std::unique_ptr<PowerUp>> powerUps;
    std::vector<StaticObject> staticObjects;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::unique_ptr<EatEffectManager> eatEffectManager;

    uint32_t totalCoins;
    uint32_t activeCoins;

    CellInfoMap mapData;
    size_t width;
    size_t height;
};

enum class GameMapState
{
    Game,
    Pause,
    Animation,
    Victory,
    GameOver,
    LevelStart
};

class GameMapProcessor
{
public:
    explicit GameMapProcessor(std::shared_ptr<GameMap> gameMap);
    ~GameMapProcessor();

    void update(float time);
    void setVisible(bool visible);
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
    float _totalTime = 0;
    float _deltaTime = 0;

    bool _isVisible = true;
};

} // namespace Chewman