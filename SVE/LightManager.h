// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "LightNode.h"
#include <vector>
#include <set>

namespace SVE
{

class VulkanShadowImage;
class VulkanPointShadowMap;
class ShadowMap;

static const uint32_t MAX_CASCADES = 5;

class LightManager
{
public:
    explicit LightManager(bool useCascadeShadowMap = false);
    ~LightManager();

    void addLight(LightNode* light);
    void removeLight(LightNode* light);

    LightNode* getLight(uint32_t index) const;
    LightNode* getDirectionLight() const;
    size_t getLightCount() const;

    std::shared_ptr<ShadowMap> getPointLightShadowMap();
    std::shared_ptr<ShadowMap> getDirectLightShadowMap();
    void setDirectShadowOrtho(glm::vec4 frame, glm::vec2 nearFar);
    std::pair<glm::vec4, glm::vec2> getDirectShadowOrtho() const;

    void setCurrentFrame(uint64_t frame);
    void fillUniformData(UniformData& data, LightType viewSourceLightType = LightType::None);

private:
    std::set<LightNode*> _lightList;

    LightNode* _directLight = nullptr;

    std::shared_ptr<ShadowMap> _pointLightShadowMap;
    std::shared_ptr<ShadowMap> _directLightShadowMap;
    bool _useCascadeShadowMap = false;
    bool _usePointLightShadow = false;

    uint64_t _currentFrame = 0;

    glm::vec4 _shadowCameraFrame = { -100.0f, 100.0f, -100.0f, 100.0f };
    glm::vec2 _shadowCameraNearFar = { 1.0f, 100.0f };
};

} // namespace SVE