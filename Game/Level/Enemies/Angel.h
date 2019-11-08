// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "DefaultEnemy.h"

namespace Chewman
{

class Angel final : public DefaultEnemy
{
public:
    Angel(GameMap* map, glm::ivec2 startPos);

    void increaseState(EnemyState state) override;
    void decreaseState(EnemyState state) override;

protected:
    float getHeight() override;

private:
    std::shared_ptr<SVE::MeshEntity> _wingsMesh;
};

} // namespace Chewman