// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include "ShaderSettings.h"

namespace SVE
{

struct ComputeSettings
{
    std::string name;

    std::string computeShaderName;
    void* data;
    size_t dataSize;
    size_t elementsCount;
};

} // namespace SVE