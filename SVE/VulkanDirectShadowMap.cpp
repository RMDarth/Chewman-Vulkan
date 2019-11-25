// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LightNode.h"
#include "VulkanDirectShadowMap.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "MaterialManager.h"
#include "VulkanMaterial.h"
#include "VulkanPassInfo.h"
#include "VulkanSamplerHolder.h"


namespace SVE
{

VulkanDirectShadowMap::VulkanDirectShadowMap(uint32_t layersCount, uint32_t shadowMapSize)
        : _layersCount(layersCount)
        , _shadowMapSize(shadowMapSize)
        , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
        , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    createRenderPass();
    createImageResources();
    createFramebuffer();

    updateSamplers();
}

VulkanDirectShadowMap::~VulkanDirectShadowMap()
{
    deleteFramebuffer();
    deleteImageResources();
    deleteRenderPass();
}

void VulkanDirectShadowMap::reallocateCommandBuffers()
{
    _commandBuffers.resize(_vulkanInstance->getInFlightSize());
    for (auto i = 0u; i < _vulkanInstance->getInFlightSize(); i++)
    {
        _commandBuffers[i] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SHADOWMAP_SUN + i);
    }
}

uint32_t VulkanDirectShadowMap::startRenderCommandBufferCreation(uint32_t bufferNumber, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffers[bufferNumber], &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[0].depthStencil = {1.0f, 0};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _shadowMapSize;
    renderPassBeginInfo.renderArea.extent.height = _shadowMapSize;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_commandBuffers[bufferNumber], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffers[bufferNumber],
            1.25f,
            0.0f,
            1.75f);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = _shadowMapSize;
    viewport.height = _shadowMapSize;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(_commandBuffers[bufferNumber], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _shadowMapSize;
    scissor.extent.height = _shadowMapSize;

    vkCmdSetScissor(_commandBuffers[bufferNumber], 0, 1, &scissor);

    return BUFFER_INDEX_SHADOWMAP_SUN + bufferNumber;
}

void VulkanDirectShadowMap::endRenderCommandBufferCreation(uint32_t bufferIndex)
{
    vkCmdEndRenderPass(_commandBuffers[bufferIndex]);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffers[bufferIndex]) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

VkSampler VulkanDirectShadowMap::getSampler(uint32_t index) const
{
    return _shadowSampler[index];
}

void VulkanDirectShadowMap::createRenderPass()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();

    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = _vulkanInstance->getSurfaceColorFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkSubpassDependency> dependencies(2);

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments { colorAttachment, depthAttachment  };

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
    _vulkanInstance->getPassInfo()->setPassData(CommandsType::ShadowPassDirectLight, data);
}

void VulkanDirectShadowMap::deleteRenderPass()
{
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass, nullptr);
}

void VulkanDirectShadowMap::createImageResources()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();

    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.minLod = 0;
    samplerCreateInfo.maxLod = 1;
    samplerCreateInfo.mipLodBias = 0;

    auto size = _vulkanInstance->getSwapchainSize();
    _shadowImage.resize(size);
    _shadowImageView.resize(size);
    _shadowImageMemory.resize(size);
    _shadowSampler.resize(size);

    for (auto i = 0; i < size; i++)
    {
        _vulkanInstance->getVulkanUtils().createImage(
                _shadowMapSize,
                _shadowMapSize,
                1,
                VK_SAMPLE_COUNT_1_BIT,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _shadowImage[i],
                _shadowImageMemory[i],
                0,
                _layersCount);

        _shadowImageView[i] = _vulkanInstance->getVulkanUtils().createImageView(
                _shadowImage[i],
                depthFormat,
                1,
                aspectFlags,
                _layersCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                _layersCount,
                0);

        _vulkanInstance->getVulkanUtils().transitionImageLayout(
                _shadowImage[i],
                depthFormat,
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
                1,
                aspectFlags,
                _layersCount);

        // Create sampler
        if (vkCreateSampler(_vulkanInstance->getLogicalDevice(), &samplerCreateInfo, nullptr, &_shadowSampler[i]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan texture sampler");
        }
    }

    // create unused color attachment
    _vulkanUtils.createImage(
            _shadowMapSize,
            _shadowMapSize,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            _vulkanInstance->getSurfaceColorFormat(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
}

void VulkanDirectShadowMap::deleteImageResources()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroySampler(_vulkanInstance->getLogicalDevice(), _shadowSampler[i], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _shadowImageView[i], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _shadowImage[i], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _shadowImageMemory[i], nullptr);
    }

    vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _colorImageView, nullptr);
    vkDestroyImage(_vulkanInstance->getLogicalDevice(), _colorImage, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _colorImageMemory, nullptr);
}

void VulkanDirectShadowMap::createFramebuffer()
{
    _framebuffers.resize(_vulkanInstance->getSwapchainSize());
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        std::vector<VkImageView> attachments = { _colorImageView, _shadowImageView[i] };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _renderPass;
        framebufferCreateInfo.attachmentCount = attachments.size();
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = _shadowMapSize;
        framebufferCreateInfo.height = _shadowMapSize;
        framebufferCreateInfo.layers = _layersCount;

        if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffers[i]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan Framebuffer");
        }
    }
}

void VulkanDirectShadowMap::deleteFramebuffer()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffers[i], nullptr);
    }
}

void VulkanDirectShadowMap::updateSamplers()
{
    VulkanSamplerInfoList samplerInfoList;
    for (auto i = 0; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        VulkanSamplerHolder::SamplerInfo samplerInfo{ _shadowImageView[i], _shadowSampler[i] };
        samplerInfoList.push_back(samplerInfo);
    }
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::ShadowMapDirect, samplerInfoList);
}

} // namespace SVE