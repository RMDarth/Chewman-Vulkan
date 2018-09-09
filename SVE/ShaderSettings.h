// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vector>
#include <string>
#include <map>
#include "Libs.h"

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

struct UniformData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraPos;
    glm::vec3 lightPos;
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

const std::map<UniformType, size_t>& getUniformSizeMap();

} // namespace SVE