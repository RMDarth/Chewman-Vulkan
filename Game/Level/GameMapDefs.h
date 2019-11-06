// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <vector>
#include <memory>

namespace SVE
{
class SceneNode;
}

namespace Chewman
{

struct Coin;
class PowerUp;
struct Teleport;

constexpr float CellSize = 3.0f;
constexpr float MoveSpeed = 6.5f;

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
    PowerUp* powerUp = nullptr;
    Teleport* teleport = nullptr;
    std::shared_ptr<SVE::SceneNode> cellBlock;
};

using CellInfoMap = std::vector<std::vector<CellInfo>>;

} // namespace Chewman