// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/Entity.h>

namespace Chewman
{
template<typename Info>
class CustomEntity;

struct FireballInfo
{
    glm::vec3 startPos;
    uint32_t maxParticles = 1500;

    glm::mat4 toMat4() const
    {
        return glm::mat4(
                glm::vec4(startPos, maxParticles),
                glm::vec4(0.0),
                glm::vec4(0.0),
                glm::vec4(0.0));
    }
};

using FireballEntity = CustomEntity<FireballInfo>;

} // namespace Chewman