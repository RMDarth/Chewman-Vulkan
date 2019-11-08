// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "GameMap.h"
#include <array>

namespace Chewman
{

class GameMapLoader
{
public:
    GameMapLoader();
    std::shared_ptr<GameMap> loadMap(const std::string& filename, const std::string& suffix = "");

private:
    void initMeshes(GameMap& level, const std::string& suffix);

    void createGargoyle(GameMap& level, int row, int column, char mapType);
    void finalizeGargoyle(GameMap& level, Gargoyle& gargoyle);

    void createTeleport(GameMap& level, int row, int column, char mapType);
    Coin* createCoin(GameMap& level, int row, int column);

    void createLava(GameMap& level, const std::string& suffix) const;
    void createSmoke(GameMap& level) const;

private:
    BlockMeshGenerator _meshGenerator;
};


} // namespace Chewman