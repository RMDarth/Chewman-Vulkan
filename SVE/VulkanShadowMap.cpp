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

namespace SVE
{

VulkanShadowMap::VulkanShadowMap()
    : _width(2048)
    , _height(2048)
    , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    createRenderPass();
    createShadowImage();
    createFramebuffer();
}

VulkanShadowMap::~VulkanShadowMap()
{
    deleteFramebuffer();
    deleteShadowImage();
    deleteRenderPass();
}

void VulkanShadowMap::setLight(std::shared_ptr<LightNode> light)
{
    _light = std::move(light);
}

VkSampler VulkanShadowMap::getShadowMapSampler()
{
    return _shadowSampler;
}

VkImageView VulkanShadowMap::getShadowMapImageView()
{
    return _shadowImageView;
}

void VulkanShadowMap::createRenderPass()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkSubpassDependency> dependencies(2);

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments { depthAttachment  };

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachments.size();
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    //renderPassCreateInfo.dependencyCount = dependencies.size();
    //renderPassCreateInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan render pass");
    }
}

void VulkanShadowMap::deleteRenderPass()
{
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass, nullptr);
}

void VulkanShadowMap::createShadowImage()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();

    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    _vulkanUtils.createImage(
            _width,
            _height,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _shadowImage,
            _shadowImageMemory);
    _shadowImageView = _vulkanUtils.createImageView(_shadowImage, depthFormat, 1, aspectFlags);

    _vulkanUtils.transitionImageLayout(
            _shadowImage,
            depthFormat,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
            1,
            aspectFlags);

    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.minLod = 0;
    samplerCreateInfo.maxLod = 1;
    samplerCreateInfo.mipLodBias = 0;

    if (vkCreateSampler(_vulkanInstance->getLogicalDevice(), &samplerCreateInfo, nullptr, &_shadowSampler) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan texture sampler");
    }
}

void VulkanShadowMap::deleteShadowImage()
{
    vkDestroySampler(_vulkanInstance->getLogicalDevice(), _shadowSampler, nullptr);
    vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _shadowImageView, nullptr);
    vkDestroyImage(_vulkanInstance->getLogicalDevice(), _shadowImage, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _shadowImageMemory, nullptr);
}

void VulkanShadowMap::createFramebuffer()
{
    std::vector<VkImageView> attachments = { _shadowImageView };

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = _renderPass;
    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = _width;
    framebufferCreateInfo.height = _height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Framebuffer");
    }
}

void VulkanShadowMap::deleteFramebuffer()
{
    vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffer, nullptr);
}

void VulkanShadowMap::reallocateCommandBuffers()
{
    _commandBuffer = _vulkanInstance->createCommandBuffer(0, SHADOWMAP_BUFFER_INDEX);
}

void VulkanShadowMap::startRenderCommandBufferCreation()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    VkClearValue clearValue {};
    clearValue.depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _width;
    renderPassBeginInfo.renderArea.extent.height = _height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffer,
            1.25f,
            0.0f,
            1.75f);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = _width;
    viewport.height = _height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _width;
    scissor.extent.height = _height;

    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void VulkanShadowMap::endRenderCommandBufferCreation()
{
    vkCmdEndRenderPass(_commandBuffer);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

} // namespace SVE