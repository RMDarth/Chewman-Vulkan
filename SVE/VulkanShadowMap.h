// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace SVE
{

class LightNode;
class Material;
class VulkanUtils;
class VulkanInstance;
class VulkanShadowImage;

class VulkanShadowMap
{
public:
    VulkanShadowMap(const VulkanShadowImage& vulkanShadowImage);
    ~VulkanShadowMap();

    void reallocateCommandBuffers();
    uint32_t startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex);
    void endRenderCommandBufferCreation(uint32_t bufferIndex);

private:
    void createFramebuffer();
    void deleteFramebuffer();

private:
    uint32_t _width;
    uint32_t _height;

    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;
    const VulkanShadowImage& _vulkanShadowImage;

    std::vector<VkFramebuffer> _framebuffers;
    std::vector<VkCommandBuffer> _commandBuffers;
};

} // namespace SVE