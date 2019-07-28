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
    // shape
    glm::mat4 toDirection;          // 16
    float angle;                    // 17
    float originRadius;             // 18

    // params
    float emissionRate;             // 19
    float minLife;                  // 20
    float maxLife;                  // 21
    float minSpeed;                 // 22
    float maxSpeed;                 // 23
    float minSize;                  // 24
    float maxSize;                  // 25
    float minRotate;                // 26
    float maxRotate;                // 27
    float _padding_2[1];            // 28

    glm::vec4 colorRangeStart = glm::vec4(1.0f);  // 32
    glm::vec4 colorRangeEnd = glm::vec4(1.0f);    // 36
};

struct ParticleData
{
    glm::vec3 position;
    float life;

    glm::vec4 color;

    glm::vec3 speed;
    float size;

    float acceleration;
    float currentRotation;
    float rotationSpeed;
    float scaleSpeed;

    //glm::vec4 colorChanger;
};

struct ParticleAffector
{
    float minAcceleration;      // 1
    float maxAcceleration;      // 2
    float minRotateSpeed;       // 3
    float maxRotateSpeed;       // 4
    float minScaleSpeed;        // 5
    float maxScaleSpeed;        // 6
    float lifeDrain;            // 7
    float _padding[1];          // 8
    glm::vec4 colorChanger;     // 12
};

struct ParticleSystemSettings
{
    std::string name;
    std::string materialName;
    std::string computeShaderName;

    uint32_t quota;
    bool sort;

    ParticleEmitter particleEmitter;
    ParticleAffector particleAffector;
};

} // namespace SVE