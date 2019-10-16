// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Nun.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{

Nun::Nun(GameMap* map, glm::ivec2 startPos)
    : DefaultEnemy(map, startPos, EnemyType::Nun,
                   "nun", "NunMaterial", 95)
{
    createMaterials();

    auto transform = glm::scale(glm::mat4(1), glm::vec3(4.0f));
    transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0, 1, 0));
    _meshNode->setNodeTransformation(transform);
}

} // namespace Chewman