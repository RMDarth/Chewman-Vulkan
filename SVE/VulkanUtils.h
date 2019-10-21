// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

#include "VulkanHeaders.h"
#include <vector>

namespace SVE
{
class VulkanInstance;

class VulkanUtils
{
public:
    struct ImageLayoutState
    {
        VkImageLayout imageLayout;
        VkAccessFlags accessFlags;
        VkPipelineStageFlags pipelineStageFlags;
    };

    VulkanUtils();
    explicit VulkanUtils(const VulkanInstance* instance);

    void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

    // This method will create fast GPU-local buffer (using transitional temporary CPU visible buffer)
    void createOptimizedBuffer(
            const void* bufferData,
            VkDeviceSize bufferSize,
            VkBuffer &buffer,
            VkDeviceMemory &deviceMemory,
            VkBufferUsageFlags usage) const;

    void createImage(uint32_t width,
                     uint32_t height,
                     uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory,
                     VkImageCreateFlags imageCreateFlags = 0,
                     uint32_t layersNum = 1) const;
    void createCubeImage(uint32_t width,
                         uint32_t height,
                         uint32_t mipLevels,
                         VkFormat format,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image,
                         VkDeviceMemory& imageMemory) const;

    VkImageView createImageView(VkImage image,
                                VkFormat format,
                                uint32_t mipLevels,
                                VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
                                VkImageViewType imageType = VK_IMAGE_VIEW_TYPE_2D,
                                uint32_t layersCount = 1,
                                uint32_t baseLayer = 0) const;

    void generateMipmaps(VkImage image,
                         VkFormat imageFormat,
                         int32_t texWidth,
                         int32_t texHeight,
                         uint32_t mipLevels) const;

    VkCommandBuffer beginRecordingCommands() const;
    void endRecordingAndSubmitCommands(VkCommandBuffer commandBuffer) const;

    void transitionImageLayout(VkImage image,
                               VkFormat format,
                               ImageLayoutState oldLayoutState,
                               ImageLayoutState newLayoutState,
                               uint32_t mipLevels,
                               VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
                               uint32_t layersCount = 1,
                               uint32_t baseLayer = 0) const;

    uint32_t findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;

private:
    const VulkanInstance* _vulkanInstance;
};

} // namespace SVE