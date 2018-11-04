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

VulkanShadowMap::VulkanShadowMap(const VulkanShadowImage& vulkanShadowImage)
        : _width(vulkanShadowImage.getSize())
        , _height(vulkanShadowImage.getSize())
        , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
        , _vulkanUtils(_vulkanInstance->getVulkanUtils())
        , _vulkanShadowImage(vulkanShadowImage)
{
    createFramebuffer();
}

VulkanShadowMap::~VulkanShadowMap()
{
    deleteFramebuffer();
}

void VulkanShadowMap::createFramebuffer()
{
    _framebuffers.resize(_vulkanInstance->getSwapchainSize());
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        std::vector<VkImageView> attachments = { _vulkanShadowImage.getImageView(i) };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _vulkanShadowImage.getRenderPass();
        framebufferCreateInfo.attachmentCount = attachments.size();
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = _width;
        framebufferCreateInfo.height = _height;
        framebufferCreateInfo.layers = _vulkanShadowImage.getLayersSize();

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
        _commandBuffers[i] = _vulkanInstance->createCommandBuffer(_vulkanShadowImage.getBufferID(i));
    }
}

uint32_t VulkanShadowMap::startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffers[bufferNumber], &beginInfo) != VK_SUCCESS)
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

    vkCmdBeginRenderPass(_commandBuffers[bufferNumber], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffers[bufferNumber],
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

    vkCmdSetViewport(_commandBuffers[bufferNumber], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _width;
    scissor.extent.height = _height;

    vkCmdSetScissor(_commandBuffers[bufferNumber], 0, 1, &scissor);

    return _vulkanShadowImage.getBufferID(bufferNumber);
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