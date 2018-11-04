// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "LightNode.h"
#include <vector>

namespace SVE
{

class VulkanShadowImage;
class ShadowMap;

static const uint32_t MAX_CASCADES = 5;

class LightManager
{
public:
    LightManager();
    ~LightManager();

    void setLight(std::shared_ptr<LightNode> light, uint32_t index);
    std::shared_ptr<LightNode> getLight(uint32_t index) const;
    std::shared_ptr<LightNode> getDirectionLight() const;
    size_t getLightCount() const;

    std::shared_ptr<ShadowMap> getPointLightShadowMap();
    std::shared_ptr<ShadowMap> getDirectLightShadowMap();

    VulkanShadowImage* getDirectLightVulkanShadowImage();
    VulkanShadowImage* getPointLightVulkanShadowImage();

    void fillUniformData(UniformData& data, int viewSource = -1);

private:
    std::vector<std::shared_ptr<LightNode>> _lightList;

    std::shared_ptr<LightNode> _directLight;

    std::shared_ptr<ShadowMap> _pointLightShadowMap;
    std::shared_ptr<ShadowMap> _directLightShadowMap;

    std::unique_ptr<VulkanShadowImage> _shadowImageDirectLight;
    std::unique_ptr<VulkanShadowImage> _shadowImagePointLights;
};

} // namespace SVE