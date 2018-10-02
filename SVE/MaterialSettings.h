// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include <vector>

namespace SVE
{

enum class TextureType : uint8_t
{
    ImageFile,
    ShadowMap,
    Reflection,
    Refraction
};

enum class TextureAddressMode : uint8_t
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge
};

enum class TextureBorderColor : uint8_t
{
    TransparentBlack,
    SolidBlack,
    SolidWhite
};

struct TextureInfo
{
    TextureType textureType = TextureType::ImageFile;
    TextureAddressMode textureAddressMode = TextureAddressMode::Repeat;
    TextureBorderColor textureBorderColor = TextureBorderColor::TransparentBlack;
    std::string samplerName;
    std::string filename;
    // TODO: Add possibility to enable/disable mipmap generation
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