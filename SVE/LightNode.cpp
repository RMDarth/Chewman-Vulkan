// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightNode.h"
#include "ShaderSettings.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

LightNode::LightNode(LightSettings lightSettings)
    : _lightSettings(std::move(lightSettings))
{

}

const LightSettings& LightNode::getLightSettings()
{
    return _lightSettings;
}

void LightNode::fillUniformData(UniformData& data)
{
    auto model = getTotalTransformation();
    data.lightPos = model[3];
    // TODO: Light should have it's own projection matrix
    data.lightViewProjection = data.projection * getViewMatrix();
    data.lightSettings = _lightSettings;
}

glm::mat4 LightNode::getViewMatrix()
{
    auto model = getTotalTransformation();
    return glm::lookAt(glm::vec3(model[3]), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
}


} // namespace SVE