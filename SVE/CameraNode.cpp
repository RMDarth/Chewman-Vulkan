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
    createMatrix();
}

CameraNode::CameraNode()
    : _cameraSettings({45.0f, 1.77f, 0.1f, 100.0f})
{
    auto extent = Engine::getInstance()->getVulkanInstance()->getExtent();
    _cameraSettings.aspectRatio = (float)extent.width / extent.height;

    createMatrix();
}

void CameraNode::setNearFarPlane(float near, float far)
{
    _cameraSettings.nearPlane = near;
    _cameraSettings.farPlane = far;

    createMatrix();
}

void CameraNode::setFOV(float fov)
{
    _cameraSettings.fieldOfView = fov;

    createMatrix();
}

void CameraNode::setAspectRatio(float aspectRatio)
{
    _cameraSettings.aspectRatio = aspectRatio;

    createMatrix();
}

const glm::mat4& CameraNode::getProjectionMatrix()
{
    return _projection;
}

void CameraNode::createMatrix()
{
    _projection =  glm::perspective(glm::radians(_cameraSettings.fieldOfView),
                                    _cameraSettings.aspectRatio,
                                    _cameraSettings.nearPlane,
                                    _cameraSettings.farPlane);
    _projection[1][1] *= -1;
}

UniformData CameraNode::fillUniformData()
{
    UniformData data;
    data.projection = getProjectionMatrix();
    data.view = getTotalTransformation();
    data.model = glm::mat4(1);

    return data;
}

void CameraNode::setLookAt(glm::vec3 pos, glm::vec3 target,  glm::vec3 up)
{
    setNodeTransformation(glm::lookAt(pos, target, up));
}


} // namespace SVE