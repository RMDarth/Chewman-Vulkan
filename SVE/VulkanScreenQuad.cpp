// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanScreenQuad.h"
#include "VulkanSamplerHolder.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "ShaderSettings.h"
#include "VulkanPassInfo.h"

namespace SVE
{

VulkanScreenQuad::VulkanScreenQuad(glm::ivec2 resolution)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _width(resolution.x > 0 ? resolution.x : _vulkanInstance->getExtent().width)
    , _height(resolution.y > 0 ? resolution.y : _vulkanInstance->getExtent().height)
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    createRenderPass();
    createImages();
    createFramebuffers();

    auto scSize = _vulkanInstance->getSwapchainSize();
    VulkanSamplerHolder::SamplerInfo samplerInfo { _resolveImageView[0], _colorSampler[0] };
    VulkanSamplerInfoList list(scSize, samplerInfo);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::ScreenQuad, list);

    VulkanSamplerHolder::SamplerInfo samplerInfoLate {_resolveImageView[1], _colorSampler[1] };
    VulkanSamplerInfoList listLate(scSize, samplerInfoLate);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::ScreenQuadSecond, listLate);

    VulkanSamplerHolder::SamplerInfo samplerInfoDepth {_depthImageView[1], _depthSampler };
    VulkanSamplerInfoList listDepth(scSize, samplerInfoDepth);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::ScreenQuadDepth, listDepth);
}

VulkanScreenQuad::~VulkanScreenQuad()
{
    deleteFramebuffers();
    deleteImages();
    deleteRenderPass();
}

VkSampler VulkanScreenQuad::getSampler()
{
    return _colorSampler[ScreenQuadPass::Late];
}

VkImageView VulkanScreenQuad::getImageView()
{
    return _resolveImageView[ScreenQuadPass::Late];
}

void VulkanScreenQuad::reallocateCommandBuffers(ScreenQuadPass screenQuadPass)
{
    switch (screenQuadPass)
    {
        case Normal:
            _commandBuffer[screenQuadPass] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SCREEN_QUAD);
            break;
        case MRT:
            _commandBuffer[screenQuadPass] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SCREEN_QUAD_MRT);
            break;
        case Late:
            _commandBuffer[screenQuadPass] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SCREEN_QUAD_LATE);
            break;
        case Depth:
            _commandBuffer[screenQuadPass] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_SCREEN_QUAD_DEPTH);
            break;
    }
}

void VulkanScreenQuad::startRenderCommandBufferCreation(ScreenQuadPass screenQuadPass)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffer[screenQuadPass], &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    std::vector<VkClearValue> clearValues(3);
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 0.0f};
    if (screenQuadPass == ScreenQuadPass::MRT)
    {
        clearValues.resize(5);
        clearValues[3].color = {0.0f, 0.0f, 0.0f, 0.0f};
        clearValues[4].color = {0.0f, 0.0f, 0.0f, 0.0f};
    }
    if (screenQuadPass == ScreenQuadPass::Depth)
        clearValues.resize(2);

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass[screenQuadPass];
    renderPassBeginInfo.framebuffer = _framebuffer[screenQuadPass];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _width;
    renderPassBeginInfo.renderArea.extent.height = _height;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_commandBuffer[screenQuadPass], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffer[screenQuadPass],
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

    vkCmdSetViewport(_commandBuffer[screenQuadPass], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _width;
    scissor.extent.height = _height;

    vkCmdSetScissor(_commandBuffer[screenQuadPass], 0, 1, &scissor);
}

void VulkanScreenQuad::endRenderCommandBufferCreation(ScreenQuadPass screenQuadPass)
{
    vkCmdEndRenderPass(_commandBuffer[screenQuadPass]);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffer[screenQuadPass]) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

void VulkanScreenQuad::createRenderPass()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();
    auto sampleCount = _vulkanInstance->getMSAASamples();

    // TODO: Refactor this code, move to separate functions

    VkAttachmentDescription colorAttachment[4] = {}; // color buffer attachment to render pass
    // resolve color attachment (after MSAA applied)
    VkAttachmentDescription colorAttachmentResolve[4] = {};

    for (auto i = 0; i < 4; ++i)
    {
        colorAttachment[i].format = _vulkanInstance->getSurfaceColorFormat();
        colorAttachment[i].samples = i < 3 ? sampleCount : VK_SAMPLE_COUNT_1_BIT;
        colorAttachment[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear every frame
        colorAttachment[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // should be STORE for rendering
        colorAttachment[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentResolve[i].format = _vulkanInstance->getSurfaceColorFormat();
        colorAttachmentResolve[i].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    colorAttachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // reuse data from previous pass
    colorAttachment[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription depthAttachment[4] = {};
    for (auto i = 0; i < 4; ++i)
    {
        depthAttachment[i].format = depthFormat;
        depthAttachment[i].samples = i < 3 ? sampleCount : VK_SAMPLE_COUNT_1_BIT;
        depthAttachment[i].loadOp = (i == 0 || i == 3) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment[i].initialLayout = (i == 0 || i == 3) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment[i].finalLayout = (i != 2 && i != 3) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }


    VkAttachmentReference colorAttachmentRef[2] = {};
    VkAttachmentReference colorAttachmentResolveRef[2] = {};
    colorAttachmentRef[0].attachment = 0;
    colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentResolveRef[0].attachment = 2;
    colorAttachmentResolveRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentRef[1].attachment = 3;
    colorAttachmentRef[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentResolveRef[1].attachment = 4;
    colorAttachmentResolveRef[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef[2] {};
    depthAttachmentRef[0].attachment = 1;
    depthAttachmentRef[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentRef[1].attachment = 0;
    depthAttachmentRef[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass[3] = {};
    for (auto i = 0; i < 2; ++i)
    {
        subpass[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass[i].colorAttachmentCount = i + 1;
        subpass[i].pColorAttachments = colorAttachmentRef; // this array correspond to fragment shader output layout
        subpass[i].pDepthStencilAttachment = &depthAttachmentRef[0]; // only one depth attachment possible
        subpass[i].pResolveAttachments = colorAttachmentResolveRef; // MSAA attachment
    }
    subpass[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass[2].colorAttachmentCount = 1;
    subpass[2].pColorAttachments = colorAttachmentRef;
    subpass[2].pDepthStencilAttachment = &depthAttachmentRef[0];

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

    // TODO: Create 4 subpasses instead of 3 passes
    {
        std::vector<VkAttachmentDescription> attachments{colorAttachment[0], depthAttachment[0],
                                                         colorAttachmentResolve[0]};

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass[0];
        //renderPassCreateInfo.dependencyCount = dependencies.size();
        //renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass[Normal]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan render pass");
        }

        VulkanPassInfo::PassData data{
                _renderPass[Normal]
        };
        _vulkanInstance->getPassInfo()->setPassData(CommandsType::ScreenQuadPass, data);
    }
    {
        std::vector<VkAttachmentDescription> attachments{colorAttachment[1], depthAttachment[1],
                                                         colorAttachmentResolve[1], colorAttachment[2], colorAttachmentResolve[2]};

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass[1];
        //renderPassCreateInfo.dependencyCount = dependencies.size();
        //renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass[MRT]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan render pass");
        }

        VulkanPassInfo::PassData data{
                _renderPass[MRT]
        };
        _vulkanInstance->getPassInfo()->setPassData(CommandsType::ScreenQuadMRTPass, data);
    }

    {
        std::vector<VkAttachmentDescription> attachments{colorAttachment[1], depthAttachment[2],
                                                         colorAttachmentResolve[1]};

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass[0];
        //renderPassCreateInfo.dependencyCount = dependencies.size();
        //renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass[Late]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan render pass");
        }

        VulkanPassInfo::PassData data{
                _renderPass[Late]
        };
        _vulkanInstance->getPassInfo()->setPassData(CommandsType::ScreenQuadLatePass, data);
    }

    {
        std::vector<VkAttachmentDescription> attachments{ colorAttachment[3], depthAttachment[3] };

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass[2];
        //renderPassCreateInfo.dependencyCount = dependencies.size();
        //renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass[Depth]) !=
            VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan render pass");
        }

        VulkanPassInfo::PassData data{
                _renderPass[Depth]
        };
        _vulkanInstance->getPassInfo()->setPassData(CommandsType::ScreenQuadDepthPass, data);
    }
}

void VulkanScreenQuad::deleteRenderPass()
{
    for (auto i = 0; i < 4; ++i)
    {
        vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass[i], nullptr);
    }
}

void VulkanScreenQuad::createImages()
{
    auto sampleCount = _vulkanInstance->getMSAASamples();
    auto depthFormat = _vulkanInstance->getDepthFormat();
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    for (auto i = 0; i < 2; ++i)
    {
        // create color attachment image
        _vulkanUtils.createImage(
                _width,
                _height,
                1,
                sampleCount,
                _vulkanInstance->getSurfaceColorFormat(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _colorImage[i],
                _colorImageMemory[i]);
        _colorImageView[i] = _vulkanUtils.createImageView(
                _colorImage[i], _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

        _vulkanUtils.transitionImageLayout(
                _colorImage[i],
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
                _resolveImage[i],
                _resolveImageMemory[i]);
        _resolveImageView[i] = _vulkanUtils.createImageView(
                _resolveImage[i], _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

        _vulkanUtils.transitionImageLayout(
                _resolveImage[i],
                _vulkanInstance->getSurfaceColorFormat(),
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                1);
    }
    // create depth attachment image
    for (auto i = 0; i < 2; ++i)
    {
        _vulkanUtils.createImage(
                _width,
                _height,
                1,
                i == 0 ? sampleCount : VK_SAMPLE_COUNT_1_BIT,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _depthImage[i],
                _depthImageMemory[i]);
        _depthImageView[i] = _vulkanUtils.createImageView(_depthImage[i], depthFormat, 1, aspectFlags);

        _vulkanUtils.transitionImageLayout(
                _depthImage[i],
                depthFormat,
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT},
                1,
                aspectFlags);
    }

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

    for (auto& colorSampler : _colorSampler)
    {
        if (vkCreateSampler(
                _vulkanInstance->getLogicalDevice(),
                &samplerCreateInfo,
                nullptr,
                &colorSampler) != VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan texture sampler");
        }
    }
    if (vkCreateSampler(
            _vulkanInstance->getLogicalDevice(),
            &samplerCreateInfo,
            nullptr,
            &_depthSampler) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan texture sampler");
    }
}

void VulkanScreenQuad::deleteImages()
{
    for (auto i = 0; i < 2; ++i)
    {
        vkDestroySampler(_vulkanInstance->getLogicalDevice(), _colorSampler[i], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _colorImageView[i], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _resolveImageView[i], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _colorImage[i], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _resolveImage[i], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _colorImageMemory[i], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _resolveImageMemory[i], nullptr);
    }

    vkDestroySampler(_vulkanInstance->getLogicalDevice(), _depthSampler, nullptr);
    for (auto i = 0; i < 2; ++i)
    {
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _depthImageView[i], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _depthImage[i], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _depthImageMemory[i], nullptr);
    }
}

void VulkanScreenQuad::createFramebuffers()
{
    std::vector<VkImageView> attachments = { _colorImageView[0], _depthImageView[0], _resolveImageView[0] };

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = _renderPass[Normal];
    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = _width;
    framebufferCreateInfo.height = _height;
    framebufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer[Normal]) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Framebuffer");
    }

    framebufferCreateInfo.renderPass = _renderPass[Late];
    if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer[Late]) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Framebuffer");
    }

    attachments.push_back(_colorImageView[1]);
    attachments.push_back(_resolveImageView[1]);
    framebufferCreateInfo.renderPass = _renderPass[MRT];
    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();

    if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer[MRT]) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Framebuffer");
    }

    attachments.clear();
    attachments.push_back(_resolveImageView[0]);
    attachments.push_back(_depthImageView[1]);
    framebufferCreateInfo.renderPass = _renderPass[Depth];
    framebufferCreateInfo.attachmentCount = attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer[Depth]) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Framebuffer");
    }
}

void VulkanScreenQuad::deleteFramebuffers()
{
    for (auto i = 0; i < 4; ++i)
    {
        vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffer[i], nullptr);
    }
}


} // namespace SVE