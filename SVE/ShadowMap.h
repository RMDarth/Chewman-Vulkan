// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>

namespace SVE
{
class LightNode;
class VulkanCommandsManager;
enum class LightType : uint8_t;

class ShadowMap
{
public:
    ShadowMap(LightType lightType, uint32_t layersCount, uint32_t shadowMapSize);
    ~ShadowMap();

    VulkanCommandsManager* getVulkanShadowMap();

private:
    std::unique_ptr<VulkanCommandsManager> _vulkanShadowMap;
};

} // namespace SVE