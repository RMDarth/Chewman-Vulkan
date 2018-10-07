// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightNode.h"
#include "ShaderSettings.h"

#include "Engine.h"
#include "SceneManager.h"
#include "CameraNode.h"

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

void LightNode::updateViewMatrix(glm::vec3 cameraPos)
{
    _viewMatrix = glm::lookAt(_originalPos + cameraPos, glm::vec3(0, 0, 0) + cameraPos, glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::fillUniformData(UniformData& data, bool asViewSource)
{
    updateViewMatrix(data.cameraPos);

    auto model = getTotalTransformation();
    data.lightPos = model[3];
    data.lightViewProjection = _projectionMatrix * _viewMatrix;
    data.lightSettings = _lightSettings;

    if (huita || asViewSource)
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
    _originalPos = glm::vec3(model[3]);
    _viewMatrix = glm::lookAt(glm::vec3(model[3]), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::createProjectionMatrix()
{
    switch (_lightSettings.lightType)
    {
        case LightType::PointLight:
            _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                                  1.0f,
                                                  0.01f,
                                                  100.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::SunLight:
            _projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 100.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::SpotLight:
            _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                                  1.0f,
                                                  0.01f,
                                                  100.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::RectLight:
            break;
    }

}

void LightNode::setNodeTransformation(glm::mat4 transform)
{
    SceneNode::setNodeTransformation(transform);
    createViewMatrix();
}




} // namespace SVE