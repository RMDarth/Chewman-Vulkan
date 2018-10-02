// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Engine.h"
#include "VulkanUtils.h"
#include "VulkanInstance.h"
#include "VulkanException.h"

namespace SVE
{

VulkanUtils::VulkanUtils(const VulkanInstance *instance)
    : _vulkanInstance(instance)
{

}

VulkanUtils::VulkanUtils()
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
{
}

void VulkanUtils::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory) const
{
    // Create buffer
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_vulkanInstance->getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan buffer");
    }

    // Allocate memory for buffer
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(_vulkanInstance->getLogicalDevice(), buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findVulkanMemoryType(memoryRequirements.memoryTypeBits, properties);

    // TODO: It's not very convenient to allocate new memory for each new buffer, as those allocations are limited
    // TODO: It can be refactored to use single allocation for many buffers (or by using VulkanMemoryAllocator lib)
    if (vkAllocateMemory(_vulkanInstance->getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw VulkanException("Can't allocate Vulkan buffer memory");
    }

    // Bind memory to buffer
    vkBindBufferMemory(_vulkanInstance->getLogicalDevice(), buffer, bufferMemory, 0);
}

void VulkanUtils::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    // copying buffer is similar to graphical commands -
    // copy commands should be added to command buffer and submitted to queue

    auto commandBuffer = beginRecordingCommands();

    VkBufferCopy copyRegion {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // copy buffer command

    endRecordingAndSubmitCommands(commandBuffer);
}

void VulkanUtils::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const
{
    auto commandBuffer = beginRecordingCommands();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endRecordingAndSubmitCommands(commandBuffer);
}

void VulkanUtils::createImage(uint32_t width,
                             uint32_t height,
                             uint32_t mipLevels,
                             VkSampleCountFlagBits numSamples,
                             VkFormat format,
                             VkImageTiling tiling,
                             VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties,
                             VkImage& image,
                             VkDeviceMemory& imageMemory) const
{
    VkImageCreateInfo imageCreateInfo {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = numSamples; // used only for attachments
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(_vulkanInstance->getLogicalDevice(), &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(_vulkanInstance->getLogicalDevice(), image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findVulkanMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_vulkanInstance->getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw VulkanException("Can't allocate Vulkan image memory");
    }

    vkBindImageMemory(_vulkanInstance->getLogicalDevice(), image, imageMemory, 0);
}

void VulkanUtils::createCubeImage(uint32_t width,
                                  uint32_t height,
                                  uint32_t mipLevels,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkImage& image,
                                  VkDeviceMemory& imageMemory) const
{
    VkImageCreateInfo imageCreateInfo {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 6;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(_vulkanInstance->getLogicalDevice(), &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan cubemap image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(_vulkanInstance->getLogicalDevice(), image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findVulkanMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_vulkanInstance->getLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw VulkanException("Can't allocate Vulkan image memory");
    }

    vkBindImageMemory(_vulkanInstance->getLogicalDevice(), image, imageMemory, 0);
}

VkImageView VulkanUtils::createImageView(VkImage image,
                                        VkFormat format,
                                        uint32_t mipLevels,
                                        VkImageAspectFlags aspectFlags,
                                        VkImageViewType imageType,
                                        uint32_t layersCount) const
{
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = imageType;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = layersCount;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(_vulkanInstance->getLogicalDevice(), &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Image view");
    }

    return imageView;
}

void VulkanUtils::generateMipmaps(
        VkImage image,
        VkFormat imageFormat,
        int32_t texWidth,
        int32_t texHeight,
        uint32_t mipLevels) const
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_vulkanInstance->getGPU(), imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("GPU does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = beginRecordingCommands();

    // Create template struct for transition commands
    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        // transfer prev image (or original) from transfer destination to transfer source
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit {};
        blit.srcOffsets[0] = {0, 0, 0}; // source region to blit
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};  // destination region to blit
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        // Blit previous mip image (or source image) to smaller mip level image
        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Transfer prev image (or original) to optimal for shader layout
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    endRecordingAndSubmitCommands(commandBuffer);
}

VkCommandBuffer VulkanUtils::beginRecordingCommands() const
{
    // Allocate command buffer for copy command
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _vulkanInstance->getCommandPool(0);
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_vulkanInstance->getLogicalDevice(), &allocInfo, &commandBuffer);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanUtils::endRecordingAndSubmitCommands(VkCommandBuffer commandBuffer) const
{
    // Finish recording
    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer to the queue for execution
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(_vulkanInstance->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    // Wait until queue finish execution all commands
    vkQueueWaitIdle(_vulkanInstance->getGraphicsQueue());

    // Free command buffer
    vkFreeCommandBuffers(_vulkanInstance->getLogicalDevice(), _vulkanInstance->getCommandPool(0), 1, &commandBuffer);
}

void VulkanUtils::transitionImageLayout(VkImage image,
                                       VkFormat format,
                                       ImageLayoutState oldLayoutState,
                                       ImageLayoutState newLayoutState,
                                       uint32_t mipLevels,
                                       VkImageAspectFlags aspectFlags,
                                       uint32_t layersCount) const
{
    // Layout transition can be achieved using memory barriers (which used for pipeline syncronization).
    // Memory barriers are pipeline command, which should be submitted to queue.

    VkCommandBuffer commandBuffer = beginRecordingCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayoutState.imageLayout;
    barrier.newLayout = newLayoutState.imageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layersCount;
    barrier.srcAccessMask = oldLayoutState.accessFlags;
    barrier.dstAccessMask = newLayoutState.accessFlags;

    vkCmdPipelineBarrier(
            commandBuffer,
            oldLayoutState.pipelineStageFlags,
            newLayoutState.pipelineStageFlags,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    endRecordingAndSubmitCommands(commandBuffer);
}

uint32_t VulkanUtils::findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(_vulkanInstance->getGPU(), &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw VulkanException("Can't find suitable Vulkan device memory");
}

VkFormat VulkanUtils::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                         VkImageTiling tiling,
                                         VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_vulkanInstance->getGPU(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw VulkanException("Can't find suitable Vulkan format");
}



} // namespace SVE