// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "LightSettings.h"
#include "SceneNode.h"

namespace SVE
{
class ShadowMap;

class LightNode : public SceneNode
{
public:
    explicit LightNode(LightSettings lightSettings);
    ~LightNode() noexcept override ;

    const glm::mat4& getViewMatrix();
    const glm::mat4& getProjectionMatrix();

    LightSettings& getLightSettings();
    void updateViewMatrix(glm::vec3 cameraPos, glm::vec3 cameraDir);
    void fillUniformData(UniformData& data, uint32_t lightNum, bool asViewSource);
    bool castShadows() const;

    void setNodeTransformation(glm::mat4 transform) override;

private:
    void createViewMatrix();

    void createProjectionMatrix();

private:
    glm::vec3 _originalPos = {};

    LightSettings _lightSettings = {};
    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;

    float _distanceFromCamera = 0.0f;
    std::vector<glm::mat4> _projectionList; // list of projections for different layers of CSM
    std::vector<glm::mat4> _viewList; // list of view matrix for different views (cubemap) of point light
};

} // namespace SVE