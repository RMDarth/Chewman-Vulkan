// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vector>
#include <string>
#include <map>
#include "Libs.h"
#include "LightSettings.h"

namespace SVE
{

enum class UniformType : uint8_t
{
    ModelMatrix,
    ViewMatrix,
    ProjectionMatrix,
    ModelViewProjectionMatrix,
    CameraPosition,
    LightPosition,
    LightColor,
    LightAmbient,
    LightDiffuse,
    LightSpecular,
    LightShininess,
    BoneMatrices
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
    glm::vec4 cameraPos;
    glm::vec4 lightPos;
    LightSettings lightSettings;
    std::vector<glm::mat4> bones;
};

struct UniformInfo
{
    UniformType uniformType;
    int uniformIndex = 0;
};

struct VertexInfo
{
    enum VertexDataType
    {
        Position =      1 << 0,
        Color =         1 << 1,
        TexCoord =      1 << 2,
        Normal =        1 << 3,
        BoneWeights =   1 << 4,
        BoneIds =       1 << 5
    };
    int vertexDataFlags = Position | Color | TexCoord | Normal | BoneWeights | BoneIds; // TODO: remove bones
};

struct ShaderSettings
{
    std::string name;
    std::string filename;
    ShaderType shaderType;
    VertexInfo vertexInfo;
    std::vector<UniformInfo> uniformList;
    std::vector<std::string> samplerNamesList;
    uint32_t maxBonesSize;

    std::string entryPoint = "main";
};

const std::map<UniformType, size_t>& getUniformSizeMap();

} // namespace SVE