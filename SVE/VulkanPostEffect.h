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

    VkRenderPass _renderPass;

    VkImage _colorImage;
    VkImage _resolveImage;
    VkImage _depthImage;
    VkDeviceMemory _colorImageMemory;
    VkDeviceMemory _resolveImageMemory;
    VkDeviceMemory _depthImageMemory;
    VkImageView _colorImageView;
    VkImageView _resolveImageView;
    VkImageView _depthImageView;
    VkSampler _colorSampler;

    VkFramebuffer _framebuffer;
    VkCommandBuffer _commandBuffer;
};

} // namespace SVE