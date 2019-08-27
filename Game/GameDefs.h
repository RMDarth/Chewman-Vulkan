// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vector>

namespace Chewman
{

class Coin;

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
};

using CellInfoMap = std::vector<std::vector<CellInfo>>;

} // namespace Chewman