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
    RectLight
};

struct LightSettings
{
    LightType lightType;
    glm::vec3 lightColor;
    float ambientStrength;
    float specularStrength;
    float diffuseStrength;
    float shininess;
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

    uint32_t lightFlags;
    float _padding[3];
};

} // namespace SVE