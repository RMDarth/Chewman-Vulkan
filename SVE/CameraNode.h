// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Libs.h"
#include "CameraSettings.h"
#include "SceneNode.h"

namespace SVE
{

class CameraNode : public SceneNode
{
public:
    explicit CameraNode(CameraSettings cameraSettings);
    CameraNode();

    void setNearFarPlane(float near, float far);
    void setFOV(float fov);
    void setAspectRatio(float aspectRatio);

    const glm::mat4& getProjectionMatrix();
    const glm::mat4& getViewMatrix();
    const CameraSettings& getCameraSettings() const;

    void setLookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up);

    void setPosition(glm::vec3 pos);
    void movePosition(glm::vec3 deltaPos);
    glm::vec3 getPosition();
    void setYawPitchRoll(glm::vec3 yawPitchRoll);
    glm::vec3 getYawPitchRoll();

    void fillUniformData(UniformData& data);

    void setNodeTransformation(glm::mat4 transform) override;
private:
    void createProjectionMatrix();

private:
    CameraSettings _cameraSettings;
    glm::vec3 _yawPitchRoll {};
    glm::vec3 _position {};
    glm::mat4 _projection;
    glm::mat4 _view;
};

} // namespace SVE