// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/Entity.h>

namespace Chewman
{
template<typename Info>
class CustomEntity;

struct MagicInfo
{
    float ratio = 1.0f;
    float speed = 3.0f;
    float radius = 0.6f;
    float particleSize = 0.15f;
    uint32_t maxParticles = 600;
    glm::vec3 color;

    glm::mat4 toMat4() const
    {
        return glm::mat4(
                glm::vec4(ratio,speed,radius,particleSize),
                glm::vec4(color,0.0),
                glm::vec4(0.0),
                glm::vec4(0.0));
    }
};

using MagicEntity = CustomEntity<MagicInfo>;

} // namespace Chewman