// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ShaderManager.h"

namespace SVE
{

void ShaderManager::registerShader(std::shared_ptr<ShaderInfo> shader)
{
    _shaderMap.insert({shader->getName(), shader});
}

std::shared_ptr<ShaderInfo> SVE::ShaderManager::getShader(const std::string& name) const
{
    auto materialIter = _shaderMap.find(name);
    if (materialIter == _shaderMap.end())
    {
        return nullptr;
    }
    return materialIter->second;
}

} // namespace SVE