// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanShadowImage.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "VulkanPassInfo.h"

namespace SVE
{

VulkanShadowImage::VulkanShadowImage(uint32_t layersCount, uint32_t shadowMapSize, CommandsType commandsType)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _commandsType(commandsType)
    , _layersCount(layersCount)
    , _shadowMapSize(shadowMapSize)
{
    createRenderPass();
    createImageResources();
}

VulkanShadowImage::~VulkanShadowImage()
{
    deleteImageResources();
    deleteRenderPass();
}

uint32_t VulkanShadowImage::getSize() const
{
    return _shadowMapSize;
}

uint32_t VulkanShadowImage::getLayersSize() const
{
    return _layersCount;
}

VkImage VulkanShadowImage::getImage(uint32_t index) const
{
    return _shadowImage[index];
}

VkImageView VulkanShadowImage::getImageView(uint32_t index) const
{
    return _shadowImageView[index];
}

VkSampler VulkanShadowImage::getSampler(uint32_t index) const
{
    return _shadowSampler[index];
}

VkRenderPass VulkanShadowImage::getRenderPass() const
{
    return _renderPass;
}

void VulkanShadowImage::createImageResources()
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
                VK_IMAGE_VIEW_TYPE_2D_ARRAY,
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
}

void VulkanShadowImage::deleteImageResources()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroySampler(_vulkanInstance->getLogicalDevice(), _shadowSampler[i], nullptr);
        vkDestroyImageView(_vulkanInstance->getLogicalDevice(), _shadowImageView[i], nullptr);
        vkDestroyImage(_vulkanInstance->getLogicalDevice(), _shadowImage[i], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _shadowImageMemory[i], nullptr);
    }
}

void VulkanShadowImage::createRenderPass()
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
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments { depthAttachment  };

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachments.size();
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = dependencies.size();
    renderPassCreateInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(_vulkanInstance->getLogicalDevice(), &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan render pass");
    }

    VulkanPassInfo::PassData data {
            _renderPass
    };
    _vulkanInstance->getPassInfo()->setPassData(_commandsType, data);
}

void VulkanShadowImage::deleteRenderPass()
{
    vkDestroyRenderPass(_vulkanInstance->getLogicalDevice(), _renderPass, nullptr);
}

uint32_t VulkanShadowImage::getBufferID(uint32_t inflightBufferNum) const
{
    switch (_commandsType)
    {
        case CommandsType::ShadowPassDirectLight:
            return BUFFER_INDEX_SHADOWMAP_SUN + inflightBufferNum;
        case CommandsType::ShadowPassPointLights:
            return BUFFER_INDEX_SHADOWMAP_POINT + inflightBufferNum;
        default:
            throw VulkanException("Incorrect pass id");
    }
}

} // namespace SVE