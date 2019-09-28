// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "GameMap.h"

namespace Chewman
{

class GameMapLoader
{
public:
    GameMapLoader();
    std::shared_ptr<GameMap> loadMap(const std::string& filename);
    void initMeshes(GameMap& level);

private:
    void createGargoyle(GameMap& level, int row, int column, char mapType);
    void finalizeGargoyle(GameMap& level, Gargoyle& gargoyle);

    void createTeleport(GameMap& level, int row, int column, char mapType);
    Coin* createCoin(GameMap& level, int row, int column);

    void createLava(GameMap& level) const;
    void createSmoke(GameMap& level) const;

private:
    BlockMeshGenerator _meshGenerator;
};

void buildLevelMeshes(const GameMap& level, BlockMeshGenerator& meshGenerator);


} // namespace Chewman