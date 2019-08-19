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

namespace Chewman
{

enum class CellType
{
    Wall,
    Floor,
    Liquid,
    InvisibleWallWithFloor,
    InvisibleWallEmpty
};

using Map = std::vector<std::vector<CellType>>;

class GameMap
{
public:
    GameMap();

    void LoadMap(const std::string& filename);
    void Update(float time);

private:
    void InitMeshes();
    void CreateGargoyle(int row, int column, char mapType);
    void FinalizeGargoyle(Gargoyle& gargoyle);

    void InitTeleportMesh();
    void CreateTeleport(int row, int column, char mapType);
    void UpdateTeleport(float time, Teleport& teleport);

private:
    BlockMeshGenerator _meshGenerator;

    std::shared_ptr<SVE::Mesh> _mapMesh[3];
    std::shared_ptr<SVE::SceneNode> _mapNode;
    std::shared_ptr<SVE::MeshEntity> _mapEntity[3];

    std::vector<Gargoyle> _gargoyles;
    std::vector<Teleport> _teleports;

    Map _mapData;
    size_t _width;
    size_t _height;
};

} // namespace Chewman