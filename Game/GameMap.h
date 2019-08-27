// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
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
#include "Enemies/Nun.h"

namespace Chewman
{

struct GameMap
{
    std::shared_ptr<SVE::SceneNode> mapNode;
    std::shared_ptr<SVE::MeshEntity> mapEntity[3];

    std::vector<Gargoyle> gargoyles;
    std::vector<Teleport> teleports;
    std::vector<Coin> coins;
    std::vector<Nun> nuns;

    CellInfoMap mapData;
    size_t width;
    size_t height;
};

class GameMapProcessor
{
public:
    explicit GameMapProcessor(std::shared_ptr<GameMap> gameMap);

    void update(float time);

private:
    void updateGargoyle(float time, Gargoyle& gargoyle);
    void updateTeleport(float time, Teleport& teleport);
    void updateCoin(float time, Coin& coin);

private:
    std::shared_ptr<GameMap> _gameMap;
};

} // namespace Chewman