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
    createProjectionMatrix();
}

const LightSettings& LightNode::getLightSettings()
{
    return _lightSettings;
}

void LightNode::fillUniformData(UniformData& data, bool asViewSource)
{
    auto model = getTotalTransformation();
    data.lightPos = model[3];
    // TODO: Light should have it's own projection matrix
    data.lightViewProjection = _projectionMatrix * _viewMatrix;
    data.lightSettings = _lightSettings;

    if (asViewSource)
    {
        data.view = _viewMatrix;
        data.projection = _projectionMatrix;
    }
}

const glm::mat4& LightNode::getViewMatrix()
{
    return _viewMatrix;
}

const glm::mat4& LightNode::getProjectionMatrix()
{
    return _projectionMatrix;
}

void LightNode::createViewMatrix()
{
    auto model = getTotalTransformation();
    _viewMatrix = glm::lookAt(glm::vec3(model[3]), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::createProjectionMatrix()
{
    _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                    1.0f,
                                    0.01f,
                                    50.0f);
    _projectionMatrix[1][1] *= -1;
}

void LightNode::setNodeTransformation(glm::mat4 transform)
{
    SceneNode::setNodeTransformation(transform);
    createViewMatrix();
}


} // namespace SVE