// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanWater.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanSamplerHolder.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "ShaderSettings.h"

#include "SceneManager.h"
#include "CameraNode.h"
#include "VulkanPassInfo.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

VulkanWater::VulkanWater(float height)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _width {300, _vulkanInstance->getExtent().width}
    , _height { static_cast<uint32_t>(300.0f * _vulkanInstance->getExtent().height / _vulkanInstance->getExtent().width),
                _vulkanInstance->getExtent().height }
    , _waterHeight(height)
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    createRenderPasses();
    createImages();
    createFramebuffers();

    VulkanSamplerHolder::SamplerInfo reflectionSamplerInfo { _resolveImageView[0], _colorSampler[0] };
    VulkanSamplerInfoList reflectionList(3, reflectionSamplerInfo);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::Reflection, reflectionList);

    VulkanSamplerHolder::SamplerInfo refractionSamplerInfo { _resolveImageView[1], _colorSampler[1] };
    VulkanSamplerInfoList refractionList(3, refractionSamplerInfo);
    _vulkanInstance->getSamplerHolder()->setSamplerInfo(TextureType::Refraction, refractionList);
}

VulkanWater::~VulkanWater()
{
    deleteFramebuffers();
    deleteImages();
    deleteRenderPasses();
}

void VulkanWater::setHeight(float height)
{
    _waterHeight = height;
}

VkSampler VulkanWater::getSampler(PassType passType)
{
    return _colorSampler[static_cast<uint8_t>(passType)];
}

VkImageView VulkanWater::getImageView(PassType passType)
{
    return _resolveImageView[static_cast<uint8_t>(passType)];
}

void VulkanWater::fillUniformData(UniformData& data, PassType passType)
{
    auto camera = Engine::getInstance()->getSceneManager()->getMainCamera();

    if (passType == PassType::Reflection)
    {
        data.clipPlane = glm::vec4(0, 1, 0, _waterHeight);

        auto pos = camera->getPosition();
        auto distance = 2 * (pos.y - _waterHeight);
        pos.y -= distance;

        auto yawPitchRoll = camera->getYawPitchRoll();

        auto cameraPos = glm::yawPitchRoll(yawPitchRoll.x, -yawPitchRoll.y, yawPitchRoll.z);
        cameraPos = glm::translate(glm::mat4(1), glm::vec3(pos)) * cameraPos;
        data.view = glm::inverse(cameraPos);

    } else {
        data.clipPlane = glm::vec4(0, -1, 0, _waterHeight);
    }
}

void VulkanWater::reallocateCommandBuffers()
{
    _commandBuffer[0] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_WATER_REFLECTION);
    _commandBuffer[1] = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_WATER_REFRACTION);
}

void VulkanWater::startRenderCommandBufferCreation(VulkanWater::PassType passType)
{
    auto passIndex = static_cast<uint8_t>(passType);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(_commandBuffer[passIndex], &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }

    std::vector<VkClearValue> clearValues(3);
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _renderPass[passIndex];
    renderPassBeginInfo.framebuffer = _framebuffer[passIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent.width = _width[passIndex];
    renderPassBeginInfo.renderArea.extent.height = _height[passIndex];
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_commandBuffer[passIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // TODO: Add depth bias constants to configuration
    vkCmdSetDepthBias(
            _commandBuffer[passIndex],
            1.25f,
            0.0f,
            1.75f);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = _width[passIndex];
    viewport.height = _height[passIndex];
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(_commandBuffer[passIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = _width[passIndex];
    scissor.extent.height = _height[passIndex];

    vkCmdSetScissor(_commandBuffer[passIndex], 0, 1, &scissor);
}

void VulkanWater::endRenderCommandBufferCreation(VulkanWater::PassType passType)
{
    auto passIndex = static_cast<uint8_t>(passType);

    vkCmdEndRenderPass(_commandBuffer[passIndex]);

    // finish recording
    if (vkEndCommandBuffer(_commandBuffer[passIndex]) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }
}

void VulkanWater::createRenderPasses()
{
    auto depthFormat = _vulkanInstance->getDepthFormat();
    auto sampleCount = _vulkanInstance->getMSAASamples();

    for (auto passIndex = 0u; passIndex < 2; ++passIndex)
    {
        VkAttachmentDescription colorAttachment {}; // color buffer attachment to render pass
        colorAttachment.format = _vulkanInstance->getSurfaceColorFormat();
        // TODO: Consider removing MSAA for refraction/reflection
        colorAttachment.samples = sampleCount;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear every frame
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // should be STORE for rendering
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        //renderPassCreateInfo.dependencyCount = dependencies.size();
        //renderPassCreateInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass[passIndex]) != VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan render pass");
        }

        VulkanPassInfo::PassData data {
                _renderPass[passIndex]
        };
        _vulkanInstance->getPassInfo()->setPassData(passIndex == 0 ? CommandsType::ReflectionPass : CommandsType::RefractionPass, data);
    }


}

void VulkanWater::deleteRenderPasses()
{
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass[0], nullptr);
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass[1], nullptr);
}

void VulkanWater::createImages()
{
    auto sampleCount = _vulkanInstance->getMSAASamples();
    auto depthFormat = _vulkanInstance->getDepthFormat();
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
        aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    for (auto passIndex = 0u; passIndex < 2; passIndex++)
    {
        // create color attachment image
        _vulkanUtils.createImage(
                _width[passIndex],
                _height[passIndex],
                1,
                sampleCount,
                _vulkanInstance->getSurfaceColorFormat(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _colorImage[passIndex],
                _colorImageMemory[passIndex]);
        _colorImageView[passIndex] = _vulkanUtils.createImageView(
                _colorImage[passIndex], _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

        _vulkanUtils.transitionImageLayout(
                _colorImage[passIndex],
                _vulkanInstance->getSurfaceColorFormat(),
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                1,
                VK_IMAGE_ASPECT_COLOR_BIT);

        // create resolve image
        _vulkanUtils.createImage(
                _width[passIndex],
                _height[passIndex],
                1,
                VK_SAMPLE_COUNT_1_BIT,
                _vulkanInstance->getSurfaceColorFormat(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _resolveImage[passIndex],
                _resolveImageMemory[passIndex]);
        _resolveImageView[passIndex] = _vulkanUtils.createImageView(
                _resolveImage[passIndex], _vulkanInstance->getSurfaceColorFormat(), 1, VK_IMAGE_ASPECT_COLOR_BIT);

        _vulkanUtils.transitionImageLayout(
                _resolveImage[passIndex],
                _vulkanInstance->getSurfaceColorFormat(),
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                1);

        // create depth attachment image
        _vulkanUtils.createImage(
                _width[passIndex],
                _height[passIndex],
                1,
                sampleCount,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _depthImage[passIndex],
                _depthImageMemory[passIndex]);
        _depthImageView[passIndex] = _vulkanUtils.createImageView(_depthImage[passIndex], depthFormat, 1, aspectFlags);

        _vulkanUtils.transitionImageLayout(
                _depthImage[passIndex],
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
                &_colorSampler[passIndex]) != VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan texture sampler");
        }
    }
}

void VulkanWater::deleteImages()
{
    for (auto passIndex = 0u; passIndex < 2; passIndex++)
    {
        vkDestroySampler(_vulkanInstance->getLogicalDevice(), _colorSampler[passIndex], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _colorImageView[passIndex], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _depthImageView[passIndex], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _resolveImageView[passIndex], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _colorImage[passIndex], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _depthImage[passIndex], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _resolveImage[passIndex], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _colorImageMemory[passIndex], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _depthImageMemory[passIndex], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _resolveImageMemory[passIndex], nullptr);
    }
}

void VulkanWater::createFramebuffers()
{
    for (auto passIndex = 0u; passIndex < 2; passIndex++)
    {
        std::vector<VkImageView> attachments = { _colorImageView[passIndex], _depthImageView[passIndex], _resolveImageView[passIndex] };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = _renderPass[passIndex];
        framebufferCreateInfo.attachmentCount = attachments.size();
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = _width[passIndex];
        framebufferCreateInfo.height = _height[passIndex];
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(_vulkanInstance->getLogicalDevice(), &framebufferCreateInfo, nullptr, &_framebuffer[passIndex]) != VK_SUCCESS)
        {
            throw VulkanException("Can't create Vulkan Framebuffer");
        }
    }
}

void VulkanWater::deleteFramebuffers()
{
    vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffer[0], nullptr);
    vkDestroyFramebuffer(_vulkanInstance->getLogicalDevice(), _framebuffer[1], nullptr);
}

} // namespace SVE