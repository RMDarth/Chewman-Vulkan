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
            { UniformType::CameraPosition, sizeof(glm::vec4) },
            { UniformType::LightPosition, sizeof(glm::vec4) },
            { UniformType::LightColor, sizeof(glm::vec4) },
            { UniformType::LightAmbient, sizeof(float) },
            { UniformType::LightDiffuse, sizeof(float) },
            { UniformType::LightSpecular, sizeof(float) },
            { UniformType::LightShininess, sizeof(float) },
            { UniformType::LightViewProjection, sizeof(glm::mat4) },
            { UniformType::BoneMatrices, sizeof(glm::mat4) }
    };
    return uniformSizeMap;
}

} // namespace SVE