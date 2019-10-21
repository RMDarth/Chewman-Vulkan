// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanCommandsManager.h"
#include "VulkanHeaders.h"
#include <memory>
#include <vector>

namespace SVE
{
class LightNode;
class Material;
class VulkanUtils;
class VulkanInstance;

class VulkanDirectShadowMap : public VulkanCommandsManager
{
public:
    VulkanDirectShadowMap(uint32_t layersCount, uint32_t shadowMapSize);
    ~VulkanDirectShadowMap();

    void reallocateCommandBuffers() override;
    uint32_t startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex) override;
    void endRenderCommandBufferCreation(uint32_t bufferIndex) override;

    VkSampler getSampler(uint32_t index) const;

private:
    void createRenderPass();
    void deleteRenderPass();
    void createImageResources();
    void deleteImageResources();
    void createFramebuffer();
    void deleteFramebuffer();

    void updateSamplers();

private:
    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;

    uint32_t _layersCount;
    uint32_t _shadowMapSize;

    std::vector<VkImage> _shadowImage;
    std::vector<VkImageView> _shadowImageView;
    std::vector<VkDeviceMemory> _shadowImageMemory;

    std::vector<VkSampler> _shadowSampler;
    VkRenderPass _renderPass;

    std::vector<VkFramebuffer> _framebuffers;
    std::vector<VkCommandBuffer> _commandBuffers;
};

} // namespace SVE