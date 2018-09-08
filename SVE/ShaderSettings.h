// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vector>
#include <string>

namespace SVE
{

enum class UniformType : uint8_t
{
    ModelMatrix,
    ViewMatrix,
    ProjectionMatrix,
    ModelViewProjectionMatrix,
    CameraPosition,
    LightPosition
    // TODO: Add light, material properties, time, other matrices types etc.
};

enum class ShaderType : uint8_t
{
    VertexShader = 0,
    FragmentShader,
    GeometryShader
};

struct UniformInfo
{
    UniformType uniformType;
    int uniformIndex = 0;
};

struct ShaderSettings
{
    std::string name;
    std::string filename;
    ShaderType shaderType;
    std::vector<UniformInfo> uniformList;
    std::vector<std::string> samplerNamesList;

    std::string entryPoint = "main";
};

} // namespace SVE