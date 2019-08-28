// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include "ShaderSettings.h"

namespace SVE
{

struct ComputeSettings
{
    std::string name;

    std::string computeShaderName;
    std::vector<uint8_t> data;
    size_t elementsCount;
};

} // namespace SVE