// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanPostEffect.h"
#include "VulkanSamplerHolder.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "ShaderSettings.h"
#include "VulkanPassInfo.h"

namespace SVE
{

VulkanPostEffect::VulkanPostEffect(int index, int width, int height)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _index(index)
    , _width(width < 0 ? _vulkanInstance->getExtent().width : width)
    , _height(height < 0 ? _vulkanInstance->getExtent().height : height)
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    createRenderPass();
    createImages();
    createFramebuffers();

    VulkanSamplerHolder::SamplerInfo samplerInfo {
        _vulkanInstance->getMSAASamples() == VK_SAMPLE_COUNT_1_BIT ? _colorImageView : _resolveImageView,
        _colorSampler };
    VulkanSamplerInfoList list(_vulkanInstance->getSwapchainSize(), samplerInfo);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::ScreenQuad, list, index);
}

VulkanPostEffect::~VulkanPostEffect()
{
    deleteFramebuffers();
    deleteImages();
    deleteRenderPass();
}

VkSampler VulkanPostEffect::getSampler()
{
    return _colorSampler;
}

VkImageView VulkanPostEffect::getImageView()
{
    return _resolveImageView;
}

VkCommandBuffer VulkanPostEffect::reallocateCommandBuffers()
{
    _commandBuffer = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SCREEN_QUAD + _index);
    return _commandBuffer;
}

void VulkanPostEffect::startRenderCommandBufferCreation()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    std::vector<VkClearValue> clearValues(3);
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _width;
    renderPassBeginInfo.renderArea.extent.height = _height;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

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

void VulkanPostEffect::endRenderCommandBufferCreation()
{
    vkCmdEndRenderPass(_commandBuffer);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

void VulkanPostEffect::createRenderPass()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();
    auto sampleCount = _vulkanInstance->getMSAASamples();

    VkAttachmentDescription colorAttachment {}; // color buffer attachment to render pass
    colorAttachment.format = _vulkanInstance->getSurfaceColorFormat();
    colorAttachment.samples = sampleCount;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear every frame
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // should be STORE for rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = sampleCount == VK_SAMPLE_COUNT_1_BIT
                                                    ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                    : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // resolve color attachment (after MSAA applied)
    VkAttachmentDescription colorAttachmentResolve {};
    colorAttachmentResolve.format = _vulkanInstance->getSurfaceColorFormat();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = sampleCount;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // this array correspond to fragment shader output layout
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // only one depth attachment possible
    if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
        subpass.pResolveAttachments = &colorAttachmentResolveRef; // MSAA attachment

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

    std::vector<VkAttachmentDescription> attachments { colorAttachment, depthAttachment, colorAttachmentResolve };
    if (sampleCount == VK_SAMPLE_COUNT_1_BIT)
        attachments.resize(2);

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

    VulkanPassInfo::PassData data {
            _renderPass
    };
    _vulkanInstance->getPassInfo()->setPassData(CommandsType::ScreenQuadPass, data);
}

void VulkanPostEffect::deleteRenderPass()
{
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass, nullptr);
}

void VulkanPostEffect::createImages()
{
    auto sampleCount = _vulkanInstance->getMSAASamples();
    auto depthFormat = _vulkanInstance->getDepthFormat();
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    // create color attachment image
    VkImageUsageFlags colorImageFlags = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (sampleCount == VK_SAMPLE_COUNT_1_BIT)
        colorImageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    _vulkanUtils.createImage(
            _width,
            _height,
            1,
            sampleCount,
            _vulkanInstance->getSurfaceColorFormat(),
            VK_IMAGE_TILING_OPTIMAL,
            colorImageFlags,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _colorImage,
            _colorImageMemory);
    _colorImageView = _vulkanUtils.createImageView(
            _colorImage, _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

    _vulkanUtils.transitionImageLayout(
            _colorImage,
            _vulkanInstance->getSurfaceColorFormat(),
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            1,
            VK_IMAGE_ASPECT_COLOR_BIT);

    // create resolve image
    _vulkanUtils.createImage(
            _width,
            _height,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            _vulkanInstance->getSurfaceColorFormat(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _resolveImage,
            _resolveImageMemory);
    _resolveImageView = _vulkanUtils.createImageView(
            _resolveImage, _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

    _vulkanUtils.transitionImageLayout(
            _resolveImage,
            _vulkanInstance->getSurfaceColorFormat(),
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
             VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            1);

    // create depth attachment image
    _vulkanUtils.createImage(
            _width,
            _height,
            1,
            sampleCount,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _depthImage,
            _depthImageMemory);
    _depthImageView = _vulkanUtils.createImageView(_depthImage, depthFormat, 1, aspectFlags);

    _vulkanUtils.transitionImageLayout(
            _depthImage,
            depthFormat,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
            1,
            aspectFlags);

    // Create sampler
    VkSamplerCreateInfo samplerCreateInfo{};
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

    if (vkCreateSampler(
            _vulkanInstance->getLogicalDevice(),
            &samplerCreateInfo,
            nullptr,
            &_colorSampler) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan texture sampler");
    }
}

void VulkanPostEffect::deleteImages()
{
    vkDestroySampler(_vulkanInstance->getLogicalDevice(), _colorSampler, nullptr);
    vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _colorImageView, nullptr);
    vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _depthImageView, nullptr);
    vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _resolveImageView, nullptr);
    vkDestroyImage(_vulkanInstance->getLogicalDevice(), _colorImage, nullptr);
    vkDestroyImage(_vulkanInstance->getLogicalDevice(), _depthImage, nullptr);
    vkDestroyImage(_vulkanInstance->getLogicalDevice(), _resolveImage, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _colorImageMemory, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _depthImageMemory, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _resolveImageMemory, nullptr);
}

void VulkanPostEffect::createFramebuffers()
{
    std::vector<VkImageView> attachments = { _colorImageView, _depthImageView, _resolveImageView };
    if (_vulkanInstance->getMSAASamples() == VK_SAMPLE_COUNT_1_BIT)
        attachments.resize(2);

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

void VulkanPostEffect::deleteFramebuffers()
{
    vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffer, nullptr);
}


} // namespace SVE