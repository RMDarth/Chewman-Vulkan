// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>
#include <unordered_map>
#include "ShaderInfo.h"

namespace SVE
{

class ShaderManager
{
public:
    void registerShader(std::shared_ptr<ShaderInfo> shader);
    std::shared_ptr<ShaderInfo> getShader(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<ShaderInfo>> _shaderMap;
};

} // namespace SVE