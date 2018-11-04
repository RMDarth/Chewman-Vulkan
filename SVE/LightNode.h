// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "LightSettings.h"
#include "SceneNode.h"

namespace SVE
{
class ShadowMap;

class LightNode : public SceneNode
{
public:
    explicit LightNode(LightSettings lightSettings, uint32_t lightIndex);

    const glm::mat4& getViewMatrix();
    const glm::mat4& getProjectionMatrix();

    const LightSettings& getLightSettings();
    void updateViewMatrix(glm::vec3 cameraPos);
    void fillUniformData(UniformData& data, uint32_t lightNum, bool asViewSource);
    bool castShadows() const;

    void setNodeTransformation(glm::mat4 transform) override;

private:
    void createViewMatrix();

    void createProjectionMatrix();

private:
    uint32_t _lightIndex;
    glm::vec3 _originalPos;

    LightSettings _lightSettings;
    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;

    std::vector<glm::mat4> _projectionList;
};

} // namespace SVE