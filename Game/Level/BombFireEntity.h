// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/Entity.h>

namespace Chewman
{
template<typename Info>
class CustomEntity;

struct BombFireInfo
{
    glm::vec3 startPos;
    float percent;
    float alpha = 1.0f;
    float radius = 8.5f;
    float halfSize = 2.0f;
    uint32_t maxParticles = 2500;

    glm::mat4 toMat4() const
    {
        return glm::mat4(
                glm::vec4(startPos, percent),
                glm::vec4(alpha, maxParticles, radius, halfSize),
                glm::vec4(0.0),
                glm::vec4(0.0));
    }
};

using BombFireEntity = CustomEntity<BombFireInfo>;

} // namespace Chewman