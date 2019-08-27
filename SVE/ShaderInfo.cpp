// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ShaderInfo.h"

namespace SVE
{
ShaderInfo::ShaderInfo(ShaderSettings shaderSettings)
        : _name(shaderSettings.name)
        , _vulkanShaderInfo(std::make_unique<VulkanShaderInfo>(std::move(shaderSettings)))
{

}

ShaderInfo::~ShaderInfo() = default;

VulkanShaderInfo* ShaderInfo::getVulkanShaderInfo()
{
    return _vulkanShaderInfo.get();
}

const std::string &ShaderInfo::getName()
{
    return _name;
}

} // namespace SVE