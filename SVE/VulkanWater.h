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

class VulkanWater
{
public:
    enum class PassType : uint8_t
    {
        Reflection = 0,
        Refraction = 1
    };

    explicit VulkanWater(float height);
    ~VulkanWater();

    void setHeight(float height);

    VkSampler getSampler(PassType passType);
    VkImageView getImageView(PassType passType);

    void fillUniformData(UniformData& data, PassType passType);

    void reallocateCommandBuffers();
    void startRenderCommandBufferCreation(PassType passType);
    void endRenderCommandBufferCreation(PassType passType);
private:
    void createRenderPasses();
    void deleteRenderPasses();
    void createImages();
    void deleteImages();
    void createFramebuffers();
    void deleteFramebuffers();

private:
    VulkanInstance* _vulkanInstance = nullptr;
    uint32_t _width[2] = {};
    uint32_t _height[2] = {};

    float _waterHeight = 0;

    const VulkanUtils& _vulkanUtils;

    VkRenderPass _renderPass[2] = {};

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

    VkFramebuffer _framebuffer[2] = {};
    VkCommandBuffer _commandBuffer[2] = {};

};

} // namespace SVE