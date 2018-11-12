// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ShaderSettings.h"
#include "Libs.h"

namespace SVE
{

const std::map<UniformType, size_t>& getUniformSizeMap()
{
    static std::map<UniformType, size_t> uniformSizeMap {
            { UniformType::ModelMatrix, sizeof(glm::mat4) },
            { UniformType::ViewMatrix, sizeof(glm::mat4) },
            { UniformType::ProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::ModelViewProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrix, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrixList, sizeof(glm::mat4) },
            { UniformType::ViewProjectionMatrixSize, sizeof(glm::ivec4) }, // float[3] padding
            { UniformType::CameraPosition, sizeof(glm::vec4) },
            { UniformType::MaterialInfo, sizeof(MaterialInfo) },
            { UniformType::LightInfo, sizeof(LightInfo) },
            { UniformType::LightSpot, sizeof(SpotLight) },
            { UniformType::LightPoint, sizeof(PointLight) },
            { UniformType::LightDirectional, sizeof(DirLight) },
            { UniformType::LightPointViewProjectionList, sizeof(glm::mat4) },
            { UniformType::LightDirectViewProjectionList, sizeof(glm::mat4) },
            { UniformType::BoneMatrices, sizeof(glm::mat4) },
            { UniformType::ClipPlane, sizeof(glm::vec4) },
            { UniformType::Time, sizeof(float) },
    };
    return uniformSizeMap;
}

} // namespace SVE