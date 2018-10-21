// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vulkan\vulkan.h>
#include <vector>

namespace SVE
{

class VulkanInstance;

// Class for creating and providing single multilayered image for shadow maps
class VulkanShadowImage
{
public:
    VulkanShadowImage(uint32_t layersCount, uint32_t shadowMapSize);
    ~VulkanShadowImage();

    uint32_t getSize() const;
    uint32_t getLayersSize() const;
    VkImage getImage(uint32_t index) const;
    VkImageView getImageView(uint32_t index) const;
    VkSampler getSampler(uint32_t index) const;
    VkRenderPass getRenderPass() const;

private:
    void createRenderPass();
    void deleteRenderPass();
    void createImageResources();
    void deleteImageResources();

private:
    VulkanInstance* _vulkanInstance;

    uint32_t _layersCount;
    uint32_t _shadowMapSize;
    std::vector<VkImage> _shadowImage;
    std::vector<VkImageView> _shadowImageView;
    std::vector<VkDeviceMemory> _shadowImageMemory;
    std::vector<VkSampler> _shadowSampler;
    VkRenderPass _renderPass;
};

} // namespace SVE