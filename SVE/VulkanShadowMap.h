// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <vulkan/vulkan.h>
#include <memory>

namespace SVE
{

class LightNode;
class Material;
class VulkanUtils;
class VulkanInstance;

class VulkanShadowMap
{
public:
    VulkanShadowMap();
    ~VulkanShadowMap();

    void setLight(std::shared_ptr<LightNode> light);

    VkSampler getShadowMapSampler();
    VkImageView getShadowMapImageView();

    void reallocateCommandBuffers();
    void startRenderCommandBufferCreation();
    void endRenderCommandBufferCreation();

private:
    void createRenderPass();
    void deleteRenderPass();
    void createShadowImage();
    void deleteShadowImage();
    void createFramebuffer();
    void deleteFramebuffer();

private:
    uint32_t _width;
    uint32_t _height;

    std::shared_ptr<LightNode> _light;
    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;

    VkRenderPass _renderPass;

    VkImage _shadowImage;
    VkDeviceMemory _shadowImageMemory;
    VkImageView _shadowImageView;
    VkSampler _shadowSampler;

    VkFramebuffer _framebuffer;
    VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
};

} // namespace SVE