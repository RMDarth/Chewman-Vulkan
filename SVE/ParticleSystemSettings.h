// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include "Libs.h"

namespace SVE
{

struct ParticleEmitter
{
    float angle;
    float emissionRate;
    float minTimeToLive;
    float maxTimeToLive;
    glm::vec3 direction;
    float minVelocity;
    float maxVelocity;
    glm::vec3 colorRangeStart = glm::vec3(1.0f);
    glm::vec3 colorRangeEnd = glm::vec3(1.0f);
};

struct ParticleAffector
{
    float scaleRate;
    bool applyScale = false;

    glm::vec4 colorRate;
    bool applyColor = false;

    glm::vec3 force;
    bool applyForce = false;
};

struct ParticleSystemSettings
{
    std::string name;
    std::string materialName;
    std::string computeShaderName;
    float width;
    float height;
    uint32_t quota;

    bool sort;

    ParticleEmitter particleEmitter;
    ParticleAffector particleAffector;
};

} // namespace SVE