// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Libs.h"
#include "CameraNode.h"
#include "VulkanInstance.h"
#include "Engine.h"
#include "ShaderSettings.h"

#include <utility>
#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

CameraNode::CameraNode(CameraSettings cameraSettings)
    : _cameraSettings(std::move(cameraSettings))
{
    createProjectionMatrix();
}

CameraNode::CameraNode()
    : _cameraSettings({45.0f, 1.77f, 0.1f, 100.0f})
{
    auto extent = Engine::getInstance()->getVulkanInstance()->getExtent();
    _cameraSettings.aspectRatio = (float)extent.width / extent.height;

    createProjectionMatrix();
}

void CameraNode::setNearFarPlane(float near, float far)
{
    _cameraSettings.nearPlane = near;
    _cameraSettings.farPlane = far;

    createProjectionMatrix();
}

void CameraNode::setFOV(float fov)
{
    _cameraSettings.fieldOfView = fov;

    createProjectionMatrix();
}

void CameraNode::setAspectRatio(float aspectRatio)
{
    _cameraSettings.aspectRatio = aspectRatio;

    createProjectionMatrix();
}

const glm::mat4& CameraNode::getProjectionMatrix()
{
    return _projection;
}

void CameraNode::createProjectionMatrix()
{
    _projection =  glm::perspective(glm::radians(_cameraSettings.fieldOfView),
                                    _cameraSettings.aspectRatio,
                                    _cameraSettings.nearPlane,
                                    _cameraSettings.farPlane);
    _projection[1][1] *= -1;
}

const glm::mat4& CameraNode::getViewMatrix()
{
    return _view;
}

void CameraNode::fillUniformData(UniformData& data)
{
    data.projection = _projection;
    data.view = _view;
    data.model = glm::mat4(1);
    data.cameraPos = getTotalTransformation()[3];
}

void CameraNode::setLookAt(glm::vec3 pos, glm::vec3 target,  glm::vec3 up)
{
    _view = glm::lookAt(pos, target, up);
    SceneNode::setNodeTransformation(glm::inverse(_view));
}

void CameraNode::setNodeTransformation(glm::mat4 transform)
{
    _view = glm::inverse(transform);
    SceneNode::setNodeTransformation(transform);
}


} // namespace SVE