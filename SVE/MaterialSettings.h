// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <string>
#include <vector>
#include "Engine.h"

namespace SVE
{

enum class TextureType : uint8_t
{
    ImageFile,
    ShadowMapDirect,
    ShadowMapPoint,
    Reflection,
    Refraction,
    ScreenQuad
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

enum class MaterialCullFace : uint8_t
{
    FrontFace,
    BackFace,
    None
};

struct TextureInfo
{
    TextureType textureType = TextureType::ImageFile;
    TextureAddressMode textureAddressMode = TextureAddressMode::Repeat;
    TextureBorderColor textureBorderColor = TextureBorderColor::TransparentBlack;
    std::string samplerName;
    std::string filename;
    uint32_t layers = 1;
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
    bool useMultisampling = true;
    MaterialCullFace cullFace = MaterialCullFace::FrontFace;
    CommandsType passType = CommandsType::MainPass; // used to select correct renderpass
};

} // namespace SVE