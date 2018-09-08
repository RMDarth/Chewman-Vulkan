// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0#include "VulkanMaterial.h"
#include "VulkanMaterial.h"
#include "VulkanException.h"
#include "VulkanInstance.h"
#include "Engine.h"
#include <fstream>
//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <cmath>

namespace SVE
{
namespace
{
std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file)
    {
        throw VulkanException("Can't open file " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // anon namespace

SVE::VulkanMaterial::VulkanMaterial(MaterialSettings materialSettings)
    : _materialSettings(std::move(materialSettings))
    , _device(Engine::getInstance()->getVulkanInstance()->getLogicalDevice())
    , _vulkanUtils(Engine::getInstance()->getVulkanInstance()->getVulkanUtils())
{
    createDescriptorSetLayout();
    createPipelineLayout();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

VulkanMaterial::~VulkanMaterial()
{
    deleteDescriptorSets();
    deleteDescriptorPool();
    deleteUniformBuffers();

    deleteTextureSampler();
    deleteTextureImageView();
    deleteTextureImage();

    deletePipelineLayout();
    deleteDescriptorSetLayout();
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanMaterial::createShaderStages() const
{
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
    _shaderModules.push_back(vertShaderModule);
    _shaderModules.push_back(fragShaderModule);
    return shaderStages;
}

void VulkanMaterial::freeShaderModules() const
{
    for (auto shaderModule : _shaderModules)
        vkDestroyShaderModule(_device, shaderModule, nullptr);
}

VkPipelineLayout VulkanMaterial::getPipelineLayout() const
{
    return _pipelineLayout;
}

VkDescriptorSet VulkanMaterial::getDescriptorSet(size_t index) const
{
    return _descriptorSets[index];
}

void VulkanMaterial::updateMatrices(MatricesUBO matrices) const
{
    uint32_t imageIndex = Engine::getInstance()->getVulkanInstance()->getCurrentImageIndex();
    void* data;
    vkMapMemory(_device, _uniformBuffersMemory[imageIndex], 0, sizeof(matrices), 0, &data);
    memcpy(data, &matrices, sizeof(matrices));
    vkUnmapMemory(_device, _uniformBuffersMemory[imageIndex]);
}

void VulkanMaterial::createPipelineLayout()
{
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(
            _device,
            &pipelineLayoutCreateInfo,
            nullptr,
            &_pipelineLayout) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan pipeline layout");
    }
}

void VulkanMaterial::deletePipelineLayout()
{
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}

void VulkanMaterial::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0; // binding in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // used for image sampling

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 2;
    descriptorSetLayoutCreateInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(
            _device,
            &descriptorSetLayoutCreateInfo,
            nullptr,
            &_descriptorSetLayout) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Descriptor Set layout");
    }

}

void VulkanMaterial::deleteDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(_device,
                                 _descriptorSetLayout,
                                 nullptr);
}

void VulkanMaterial::createTextureImage()
{
    // Load image pixel data
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/trashman_tex.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    _mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("Can't load texture");
    }

    // Create temporary buffer to hold pixel data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _vulkanUtils.createBuffer(imageSize,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuffer,
                              stagingBufferMemory);

    // Copy pixel data into temporary buffer
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(_device, stagingBufferMemory);

    // Free pixel data
    stbi_image_free(pixels);

    // Create texture image which will be used in shaders
    _vulkanUtils.createImage(texWidth,
                             texHeight,
                             _mipLevels,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             _textureImage,
                             _textureImageMemory);

    // Transition layout of the image to be optimal as a transfer destination
    _vulkanUtils.transitionImageLayout(
            _textureImage,
            VK_FORMAT_R8G8B8A8_UNORM,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
            _mipLevels);
    // Copy image data from buffer to image
    _vulkanUtils.copyBufferToImage(stagingBuffer,
                                   _textureImage,
                                   static_cast<uint32_t>(texWidth),
                                   static_cast<uint32_t>(texHeight));
    // Transition image layout from transfer destination to optimal for shader usage
    /*transitionImageLayout(_textureImage,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
                          {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
                          _mipLevels);*/
    _vulkanUtils.generateMipmaps(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, _mipLevels);

    // Free temporary buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void VulkanMaterial::deleteTextureImage()
{
    vkDestroyImage(_device, _textureImage, nullptr);
    vkFreeMemory(_device, _textureImageMemory, nullptr);
}

void VulkanMaterial::createTextureImageView()
{
    _textureImageView = _vulkanUtils.createImageView(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, _mipLevels);
}

void VulkanMaterial::deleteTextureImageView()
{
    vkDestroyImageView(_device, _textureImageView, nullptr);
}

void VulkanMaterial::createTextureSampler()
{
    VkSamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.minLod = 0;
    samplerCreateInfo.maxLod = _mipLevels;
    samplerCreateInfo.mipLodBias = 0;

    if (vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan texture sampler");
    }
}

void VulkanMaterial::deleteTextureSampler()
{
    vkDestroySampler(_device, _textureSampler, nullptr);
}

void VulkanMaterial::createUniformBuffers()
{
    auto swapchainSize = Engine::getInstance()->getVulkanInstance()->getSwapchainSize();
    VkDeviceSize bufferSize = sizeof(MatricesUBO);

    _uniformBuffers.resize(swapchainSize);
    _uniformBuffersMemory.resize(swapchainSize);

    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        _vulkanUtils.createBuffer(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                _uniformBuffers[i],
                _uniformBuffersMemory[i]);
    }
}

void VulkanMaterial::deleteUniformBuffers()
{
    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }
}


void VulkanMaterial::createDescriptorPool()
{
    auto swapchainSize = Engine::getInstance()->getVulkanInstance()->getSwapchainSize();

    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = _uniformBuffers.size();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapchainSize;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = swapchainSize;

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan descriptor pool");
    }
}

void VulkanMaterial::deleteDescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanMaterial::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(_uniformBuffers.size(), _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = _uniformBuffers.size();
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(_uniformBuffers.size());
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't allocate Vulkan descriptor sets");
    }

    for (auto i = 0u; i < _uniformBuffers.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MatricesUBO);

        VkDescriptorImageInfo imageInfo {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = _textureImageView;
        imageInfo.sampler = _textureSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanMaterial::deleteDescriptorSets()
{

}

VkShaderModule VulkanMaterial::createShaderModule(const std::vector<char> &code) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Shader module!");
    }

    return shaderModule;
}

} // namespace SVE