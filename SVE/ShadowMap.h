// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>

namespace SVE
{
class LightNode;
class VulkanShadowMap;

class ShadowMap
{
public:
    ShadowMap();
    ~ShadowMap();

    VulkanShadowMap* getVulkanShadowMap();
    void enableShadowMap();
    bool isEnabled();

private:
    bool _isEnabled;
    std::unique_ptr<VulkanShadowMap> _vulkanShadowMap;
};

} // namespace SVE