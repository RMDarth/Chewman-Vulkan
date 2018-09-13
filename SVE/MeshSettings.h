// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <string>
#include <vector>
#include "Libs.h"

namespace SVE
{

struct MeshSettings
{
    std::string name;
    std::vector<glm::vec3> vertexPosData;
    std::vector<glm::vec3> vertexColorData;
    std::vector<glm::vec2> vertexTexData;
    std::vector<glm::vec3> vertexNormalData;
    std::vector<uint32_t> indexData;

    std::string materialName;
};

} // namespace SVE