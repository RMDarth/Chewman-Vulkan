// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include <memory>
#include <SVE/SceneNode.h>
#include <SVE/MeshEntity.h>
#include "SVE/Mesh.h"
#include "BlockMeshGenerator.h"
#include "Gargoyle.h"
#include "Teleport.h"
#include "Coin.h"


namespace Chewman
{

enum class CellType : uint8_t
{
    Wall,
    Floor,
    Liquid,
    InvisibleWallWithFloor,
    InvisibleWallEmpty
};

struct CellInfo
{
    CellType cellType;
    Coin* coin = nullptr;
};

using Map = std::vector<std::vector<CellInfo>>;

class GameMap
{
public:
    GameMap();

    void loadMap(const std::string& filename);
    void update(float time);

private:
    void initMeshes();
    void createGargoyle(int row, int column, char mapType);
    void finalizeGargoyle(Gargoyle& gargoyle);

    void initTeleportMesh();
    void createTeleport(int row, int column, char mapType);
    void updateTeleport(float time, Teleport& teleport);

    Coin* createCoin(int row, int column);
    void updateCoin(float time, Coin& coin);

private:
    BlockMeshGenerator _meshGenerator;

    std::shared_ptr<SVE::Mesh> _mapMesh[3];
    std::shared_ptr<SVE::SceneNode> _mapNode;
    std::shared_ptr<SVE::MeshEntity> _mapEntity[3];

    std::vector<Gargoyle> _gargoyles;
    std::vector<Teleport> _teleports;
    std::vector<Coin> _coins;

    Map _mapData;
    size_t _width;
    size_t _height;
};

} // namespace Chewman