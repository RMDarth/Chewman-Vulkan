// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Nun.h"

namespace Chewman
{

Nun::Nun(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Nun,
                   "nun", "NunMaterial", 95)
{
    createMaterials();
}

} // namespace Chewman