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
    VulkanShadowMap(uint32_t lightIndex, const VulkanShadowImage& vulkanShadowImage);
    ~VulkanShadowMap();

    void reallocateCommandBuffers();
    uint32_t startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex);
    void endRenderCommandBufferCreation(uint32_t bufferIndex);

    uint32_t getBufferID(uint32_t bufferNum) const;

private:
    void createShadowImageView();
    void deleteShadowImageView();
    void createFramebuffer();
    void deleteFramebuffer();

private:
    uint32_t _lightIndex;

    uint32_t _width;
    uint32_t _height;

    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;
    const VulkanShadowImage& _vulkanShadowImage;

    std::vector<VkImageView> _shadowImageViews;
    std::vector<VkFramebuffer> _framebuffers;
    std::vector<VkCommandBuffer> _commandBuffers;
};

} // namespace SVE