// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>

namespace SVE
{
class LightNode;
class VulkanShadowMap;
enum class LightType : uint8_t;

class ShadowMap
{
public:
    explicit ShadowMap(LightType lightType);
    ~ShadowMap();

    VulkanShadowMap* getVulkanShadowMap();

private:
    std::unique_ptr<VulkanShadowMap> _vulkanShadowMap;
};

} // namespace SVE