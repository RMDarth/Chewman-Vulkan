// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Witch.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

Witch::Witch(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Nun,
                   "witch", "WitchMaterial", 95)
{
    createMaterials();

    auto transform = glm::scale(glm::mat4(1), glm::vec3(20.0f));
    //transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);
}

} // namespace Chewman