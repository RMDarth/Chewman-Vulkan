// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Libs.h"

namespace SVE
{

enum class LightType : uint8_t
{
    PointLight,
    SunLight,
    SpotLight,
    RectLight,
    LineLight,
    None
};

struct LightSettings
{
    LightType lightType;
    glm::vec3 lightColor = {1.0f, 1.0f, 1.0f};
    float ambientStrength;
    float specularStrength;
    float diffuseStrength;
    float shininess;

    glm::vec3 secondPoint = {};
    float constAtten = 1.0f * 0.05f;
    float linearAtten = 1.35f * 0.05f;
    float quadAtten = 0.44f * 0.05f;

    bool castShadows = true;
};

struct DirLight
{
    glm::vec4 direction;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

struct PointLight
{
    glm::vec4 position;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float constant;
    float linear;
    float quadratic;
    float _padding;
};

struct SpotLight
{
    glm::vec4 position;
    glm::vec4 direction;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;
    float _padding[3];
};

struct LineLight
{
    glm::vec4 startPosition;
    glm::vec4 endPosition;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float constant;
    float linear;
    float quadratic;
    float _padding;
};

struct LightInfo
{
    enum LightFlags {
        DirectionalLight =   1 << 0,
        PointLight1 =        1 << 1,
        PointLight2 =        1 << 2,
        PointLight3 =        1 << 3,
        PointLight4 =        1 << 4,
        SpotLight =          1 << 5,
    };

    uint32_t lightFlags = 0;
    uint32_t lightShadowFlags = 0;
    uint32_t lightLineNum = 0;
    float _padding[1];
};

} // namespace SVE