// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightNode.h"
#include "VulkanShadowMap.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "MaterialManager.h"
#include "VulkanMaterial.h"
#include "VulkanShadowImage.h"


namespace SVE
{

static const std::string ShadowSamplerName = "shadowmap";

VulkanShadowMap::VulkanShadowMap(uint32_t lightIndex, const VulkanShadowImage& vulkanShadowImage)
        : _lightIndex(lightIndex)
        , _width(vulkanShadowImage.getSize())
        , _height(vulkanShadowImage.getSize())
        , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
        , _vulkanUtils(_vulkanInstance->getVulkanUtils())
        , _vulkanShadowImage(vulkanShadowImage)
{
    createShadowImageView();
    createFramebuffer();
}

VulkanShadowMap::~VulkanShadowMap()
{
    deleteFramebuffer();
    deleteShadowImageView();
}

void VulkanShadowMap::createShadowImageView()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();

    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _shadowImageViews.resize(_vulkanInstance->getSwapchainSize());
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        _shadowImageViews[i] = _vulkanUtils.createImageView(
                _vulkanShadowImage.getImage(i), depthFormat, 1, aspectFlags, VK_IMAGE_VIEW_TYPE_2D, 1, _lightIndex);
    }
}

void VulkanShadowMap::deleteShadowImageView()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _shadowImageViews[i], nullptr);
    }
}

void VulkanShadowMap::createFramebuffer()
{
    _framebuffers.resize(_vulkanInstance->getSwapchainSize());
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        std::vector<VkImageView> attachments = { _shadowImageViews[i] };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _vulkanShadowImage.getRenderPass();
        framebufferCreateInfo.attachmentCount = attachments.size();
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = _width;
        framebufferCreateInfo.height = _height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffers[i]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan Framebuffer");
        }
    }
}

void VulkanShadowMap::deleteFramebuffer()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffers[i], nullptr);
    }
}

void VulkanShadowMap::reallocateCommandBuffers()
{
    _commandBuffers.resize(_vulkanInstance->getInFlightSize());
    for (auto i = 0u; i < _vulkanInstance->getInFlightSize(); i++)
    {
        // TODO: maybe just shift instead of multiplying
        _commandBuffers[i] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SHADOWMAP + i * _vulkanShadowImage.getLayersSize());
    }
}

uint32_t VulkanShadowMap::startRenderCommandBufferCreation(uint32_t bufferIndex, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffers[bufferIndex], &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    VkClearValue clearValue {};
    clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValue.depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _vulkanShadowImage.getRenderPass();
    renderPassBeginInfo.framebuffer = _framebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _width;
    renderPassBeginInfo.renderArea.extent.height = _height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(_commandBuffers[bufferIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffers[bufferIndex],
            5.25f,
            0.0f,
            5.75f);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = _width;
    viewport.height = _height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(_commandBuffers[bufferIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _width;
    scissor.extent.height = _height;

    vkCmdSetScissor(_commandBuffers[bufferIndex], 0, 1, &scissor);

    return BUFFER_INDEX_SHADOWMAP + bufferIndex * _vulkanShadowImage.getLayersSize();
}

void VulkanShadowMap::endRenderCommandBufferCreation(uint32_t bufferIndex)
{
    vkCmdEndRenderPass(_commandBuffers[bufferIndex]);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffers[bufferIndex]) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

} // namespace SVE