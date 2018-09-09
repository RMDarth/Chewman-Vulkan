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
            { UniformType::CameraPosition, sizeof(glm::vec3) },
            { UniformType::LightPosition, sizeof(glm::vec3) }
    };
    return uniformSizeMap;
}

} // namespace SVE