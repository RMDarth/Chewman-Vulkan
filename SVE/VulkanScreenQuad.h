// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <vulkan/vulkan.h>
#include <memory>

namespace SVE
{
class VulkanUtils;
class VulkanInstance;
struct UniformData;

class VulkanScreenQuad
{
public:
    VulkanScreenQuad();
    ~VulkanScreenQuad();

    VkSampler getSampler();
    VkImageView getImageView();

    void reallocateCommandBuffers(bool MRT);
    void startRenderCommandBufferCreation(bool MRT);
    void endRenderCommandBufferCreation(bool MRT);
private:
    void createRenderPass();
    void deleteRenderPass();
    void createImages();
    void deleteImages();
    void createFramebuffers();
    void deleteFramebuffers();

private:
    VulkanInstance* _vulkanInstance;
    uint32_t _width;
    uint32_t _height;

    const VulkanUtils& _vulkanUtils;

    VkRenderPass _renderPass[2];

    VkImage _colorImage[2];
    VkImage _resolveImage[2];
    VkImage _depthImage;
    VkDeviceMemory _colorImageMemory[2];
    VkDeviceMemory _resolveImageMemory[2];
    VkDeviceMemory _depthImageMemory;
    VkImageView _colorImageView[2];
    VkImageView _resolveImageView[2];
    VkImageView _depthImageView;
    VkSampler _colorSampler[2];

    VkFramebuffer _framebuffer[2];
    VkCommandBuffer _commandBuffer[2];
};

} // namespace SVE