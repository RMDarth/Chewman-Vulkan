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

} //