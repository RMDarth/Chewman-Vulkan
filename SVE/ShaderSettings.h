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
    ViewProjectionMatrix,
    ModelViewProjectionMatrix,
    CameraPosition,
    MaterialInfo,
    LightInfo,
    LightDirectional,
    LightPoint,
    LightSpot,
    LightViewProjection,
    BoneMatrices,
    ClipPlane,
    Time
    // TODO: Add material properties, other matrices types etc.
};

enum class ShaderType : uint8_t
{
    VertexShader = 0,
    FragmentShader,
    GeometryShader
};

struct MaterialInfo
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
    float _padding[3];
};



struct UniformData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    std::vector<glm::mat4> lightViewProjectionList;
    glm::vec4 cameraPos;
    glm::vec4 clipPlane;  // (Nx, Ny, Nz, DistanceFromOrigin)
    float time;
    MaterialInfo materialInfo;
    DirLight dirLight;
    std::vector<PointLight> pointLightList;
    SpotLight spotLight;
    LightInfo lightInfo;
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
    uint32_t vertexDataFlags = Position | Color | TexCoord | Normal | BoneWeights | BoneIds; // TODO: remove bones
};

struct ShaderSettings
{
    std::string name;
    std::string filename;
    ShaderType shaderType;
    VertexInfo vertexInfo;
    std::vector<UniformInfo> uniformList;
    std::vector<std::string> samplerNamesList;
    uint32_t maxBonesSize = 0;
    uint32_t maxPointLightSize = 4;
    uint32_t maxLightSize = 6;

    std::string entryPoint = "main";
};

const std::map<UniformType, size_t>& getUniformSizeMap();

} // namespace SVE