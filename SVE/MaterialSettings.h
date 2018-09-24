// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include <vector>

namespace SVE
{

struct TextureInfo
{
    std::string samplerName;
    std::string filename;
};

struct MaterialSettings
{
    std::string name;

    std::string vertexShaderName;
    std::string fragmentShaderName;
    std::string geometryShaderName;

    std::vector<TextureInfo> textures;
    bool isCubemap = false;
    bool useDepthTest = true;
    bool useDepthBias = false;
    bool invertCullFace = false;
    bool useMultisampling = true;
};

} // namespace SVE