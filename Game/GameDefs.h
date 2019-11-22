// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>

namespace Chewman
{

constexpr uint16_t LevelsCount = 36;

enum class GameState
{
    MainMenu,
    Level,
    Pause,
    Score,
    WorldSelection,
    LevelSelection,
    Graphics,
    Tutorial
};

} // namespace Chewman