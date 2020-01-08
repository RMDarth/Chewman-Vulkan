// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanHeaders.h"
#include <memory>

namespace SVE
{
class VulkanUtils;
class VulkanInstance;
struct UniformData;

class VulkanPostEffect
{
public:
    VulkanPostEffect(int index, int width = -1, int height = -1);
    ~VulkanPostEffect();

    VkSampler getSampler();
    VkImageView getImageView();

    VkCommandBuffer reallocateCommandBuffers();
    void startRenderCommandBufferCreation();
    void endRenderCommandBufferCreation();
private:
    void createRenderPass();
    void deleteRenderPass();
    void createImages();
    void deleteImages();
    void createFramebuffers();
    void deleteFramebuffers();

private:
    VulkanInstance* _vulkanInstance;
    uint32_t _index;
    uint32_t _width;
    uint32_t _height;

    const VulkanUtils& _vulkanUtils;

    VkRenderPass _renderPass = VK_NULL_HANDLE;

    VkImage _colorImage = VK_NULL_HANDLE;
    VkImage _resolveImage = VK_NULL_HANDLE;
    VkImage _depthImage = VK_NULL_HANDLE;
    VkDeviceMemory _colorImageMemory = VK_NULL_HANDLE;
    VkDeviceMemory _resolveImageMemory = VK_NULL_HANDLE;
    VkDeviceMemory _depthImageMemory = VK_NULL_HANDLE;
    VkImageView _colorImageView = VK_NULL_HANDLE;
    VkImageView _resolveImageView = VK_NULL_HANDLE;
    VkImageView _depthImageView = VK_NULL_HANDLE;
    VkSampler _colorSampler = VK_NULL_HANDLE;

    VkFramebuffer _framebuffer = VK_NULL_HANDLE;
    VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
};

} // namespace SVE