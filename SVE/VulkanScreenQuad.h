// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanHeaders.h"
#include <memory>
#include <glm/glm.hpp>

namespace SVE
{
class VulkanUtils;
class VulkanInstance;
struct UniformData;

constexpr size_t ScreenQuadBufferCount = 4;

class VulkanScreenQuad
{
public:
    enum ScreenQuadPass : uint8_t
    {
        Normal = 0,
        MRT,
        Late,
        Depth
    };

    VulkanScreenQuad(glm::ivec2 resolution);
    ~VulkanScreenQuad();

    VkSampler getSampler();
    VkImageView getImageView();

    void reallocateCommandBuffers(ScreenQuadPass screenQuadPass);
    void startRenderCommandBufferCreation(ScreenQuadPass screenQuadPass);
    void endRenderCommandBufferCreation(ScreenQuadPass screenQuadPass);
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

    VkRenderPass _renderPass[ScreenQuadBufferCount] = {};

    VkImage _colorImage[2] = {};
    VkImage _resolveImage[2] = {};
    VkImage _depthImage[2] = {};
    VkDeviceMemory _colorImageMemory[2] = {};
    VkDeviceMemory _resolveImageMemory[2] = {};
    VkDeviceMemory _depthImageMemory[2] = {};
    VkImageView _colorImageView[2] = {};
    VkImageView _resolveImageView[2] = {};
    VkImageView _depthImageView[2] = {};
    VkSampler _colorSampler[2] = {};
    VkSampler _depthSampler = VK_NULL_HANDLE;

    VkFramebuffer _framebuffer[ScreenQuadBufferCount] = {};
    VkCommandBuffer _commandBuffer[ScreenQuadBufferCount] = {};
};

} // namespace SVE