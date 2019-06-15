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

namespace Chewman
{

enum class CellType
{
    Wall,
    Floor,
    Liquid
};

using Map = std::vector<std::vector<CellType>>;

class GameMap
{
public:
    GameMap();

    void LoadMap(const std::string& filename);

private:
    void InitMeshes();

private:
    BlockMeshGenerator _meshGenerator;

    std::shared_ptr<SVE::Mesh> _mapMesh[3];

    std::shared_ptr<SVE::SceneNode> _mapNode;
    std::shared_ptr<SVE::MeshEntity> _mapEntity[3];

    Map _mapData;
    size_t _width;
    size_t _height;
};

} // namespace Chewman