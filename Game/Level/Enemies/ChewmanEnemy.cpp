// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ChewmanEnemy.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

ChewmanEnemy::ChewmanEnemy(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Chewman,
            "trashman", "BlueChewmanMaterial",
            95, 2.5f)
{
    createMaterials();
    _meshNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0, 1, 0)));
}

} // namespace Chewman