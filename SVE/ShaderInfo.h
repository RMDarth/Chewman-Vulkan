// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "ShaderSettings.h"
#include "VulkanShaderInfo.h"
#include <memory>

namespace SVE
{

class ShaderInfo
{
public:
    explicit ShaderInfo(ShaderSettings shaderSettings);
    ~ShaderInfo();

    const std::string& getName();
    VulkanShaderInfo* getVulkanShaderInfo();

private:
    std::string _name;
    std::unique_ptr<VulkanShaderInfo> _vulkanShaderInfo;
};

} // namespace SVE