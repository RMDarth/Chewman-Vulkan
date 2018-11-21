// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Libs.h"
#include "CameraNode.h"
#include "VulkanInstance.h"
#include "Engine.h"
#include "ShaderSettings.h"

#include <utility>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
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

const CameraSettings& CameraNode::getCameraSettings() const
{
    return _cameraSettings;
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
    auto cameraPos = glm::yawPitchRoll(_yawPitchRoll.x, _yawPitchRoll.y, _yawPitchRoll.z);
    cameraPos = glm::translate(glm::mat4(1), _position) * cameraPos;
    _view = glm::inverse(cameraPos);
    SceneNode::setNodeTransformation(cameraPos);

    data.projection = _projection;
    data.view = _view;
    data.model = glm::mat4(1);
    data.cameraPos = glm::vec4(_position, 1.0f);//getTotalTransformation()[3];
}

void CameraNode::setLookAt(glm::vec3 pos, glm::vec3 target,  glm::vec3 up)
{
    auto view = glm::lookAt(pos, target, up);
    SceneNode::setNodeTransformation(glm::inverse(view));
    _position = pos;
    //glm::transpose(view);
    _yawPitchRoll.x = atan2f(view[1][0], view[0][0]);
    _yawPitchRoll.y= atan2f(view[2][0], sqrtf(view[2][1] * view[2][1] + view[2][2] * view[2][2])) + 3.14f;
    //_yawPitchRoll.z= atan2(view[2][1],view[2][2]);

}

void CameraNode::setNodeTransformation(glm::mat4 transform)
{
    _view = glm::inverse(transform);
    SceneNode::setNodeTransformation(transform);
}

void CameraNode::setPosition(glm::vec3 pos)
{
    _position = pos;
}

glm::vec3 CameraNode::getPosition()
{
    return _position;
}

void CameraNode::setYawPitchRoll(glm::vec3 yawPitchRoll)
{
    _yawPitchRoll = yawPitchRoll;
}

glm::vec3 CameraNode::getYawPitchRoll()
{
    return _yawPitchRoll;
}

void CameraNode::movePosition(glm::vec3 deltaPos)
{
    auto cameraPos = glm::yawPitchRoll(_yawPitchRoll.x, _yawPitchRoll.y, _yawPitchRoll.z);
    _position += glm::vec3(cameraPos * glm::vec4(deltaPos, 0));

}


} // namespace SVE