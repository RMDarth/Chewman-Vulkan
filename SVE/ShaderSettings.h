// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <vector>
#include <string>
#include <map>
#include "Libs.h"
#include "LightSettings.h"
#include "ParticleSystemSettings.h"
#include "TextSettings.h"
#include "OverlaySettings.h"

namespace SVE
{

enum class UniformType : uint8_t
{
    ModelMatrix,
    ViewMatrix,
    ProjectionMatrix,
    InverseModelMatrix,
    ViewProjectionMatrix,
    ViewProjectionMatrixList, // for cases when several viewprojection matrices are used (cascade shadows, point lights)
    ViewProjectionMatrixSize,
    ModelViewProjectionMatrix,
    CameraPosition,
    MaterialInfo,
    LightInfo,
    LightDirectional,
    LightPoint,
    LightPointSimple,
    LightSpot,
    LightLine,
    LightPointViewProjectionList,
    LightDirectViewProjectionList,
    LightDirectViewProjection,
    BoneMatrices,
    ClipPlane,
    ParticleEmitter,
    ParticleAffector,
    ParticleCount,
    SpritesheetSize,
    ImageSize,
    TextInfo,
    GlyphInfoList,
    TextSymbolList, // move to Buffer
    OverlayInfo,
    CustomFloat,
    CustomVec4,
    CustomMat4,
    Time,
    DeltaTime
};

enum class BufferType : uint8_t
{
    AtomicCounter,
    ModelMatrixList,
    TextSymbolList
};

enum class ShaderType : uint8_t
{
    VertexShader = 0,
    FragmentShader,
    GeometryShader,
    ComputeShader
};

struct MaterialInfo
{
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
    uint32_t ignoreShadow;
    float _padding[2];
};

struct StorageData
{
    std::vector<glm::mat4> modelList;
};

struct UniformData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    std::vector<glm::mat4> viewProjectionList;
    std::vector<glm::mat4> lightDirectViewProjectionList;
    std::vector<glm::mat4> lightPointViewProjectionList;
    glm::vec4 cameraPos;
    glm::vec4 clipPlane;  // (Nx, Ny, Nz, DistanceFromOrigin)
    float time = 0;
    float deltaTime = 0;
    MaterialInfo materialInfo {};
    DirLight dirLight {};
    std::vector<PointLight> shadowPointLightList;
    std::vector<PointLight> pointLightList;
    std::vector<LineLight> lineLightList;
    SpotLight spotLight {};
    LightInfo lightInfo {};
    std::vector<glm::mat4> bones;

    ParticleEmitter particleEmitter {};
    ParticleAffector particleAffector {};
    uint32_t particleCount = 0;
    glm::ivec2 spritesheetSize;
    glm::ivec4 imageSize;

    UniformTextInfo textInfo {};
    UniformOverlayInfo overlayInfo {};
    std::vector<GlyphInfo> glyphList;
    std::vector<TextSymbolInfo> textSymbolList;

    float customFloat;
    glm::vec4 customVec4;
    glm::mat4 customMat4;
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
        BoneIds =       1 << 5,
        Custom =        1 << 6,
        Binormal =      1 << 7,
        Tangent =       1 << 8,
    };
    uint32_t vertexDataFlags = Position | Color | TexCoord | Normal | Binormal | Tangent | BoneWeights | BoneIds; // TODO: remove bones
    uint8_t positionSize = 3;
    uint8_t colorSize = 3;
    uint8_t customCount = 0;
    bool separateBinding = true;
};

struct ShaderSettings
{
    std::string name;
    std::string filename;
    ShaderType shaderType;
    VertexInfo vertexInfo;
    std::vector<UniformInfo> uniformList;
    std::vector<std::string> samplerNamesList;
    std::vector<BufferType> bufferList; // currently only supported in compute shaders
    uint32_t maxBonesSize = 0;
    uint32_t maxShadowPointLightSize = 4;
    uint32_t maxPointLightSize = 20;
    uint32_t maxLineLightSize = 15;
    uint32_t maxLightSize = 6;
    uint32_t maxCascadeLightSize = 5;
    uint32_t maxViewProjectionMatrices = 6 * maxShadowPointLightSize;
    uint32_t maxGlyphCount = 300;
    uint32_t maxTextSize = 100;

    std::string entryPoint = "main";
};

const std::map<UniformType, size_t>& getUniformSizeMap();
const std::map<BufferType, size_t>& getStorageBufferSizeMap();
std::vector<char> getUniformDataByType(const UniformData& data, UniformType type);
std::vector<char> getStorageDataByType(const StorageData& data, BufferType type);
void updateStorageDataByUniforms(const UniformData& data, StorageData& storageData, BufferType type);

} // namespace SVE