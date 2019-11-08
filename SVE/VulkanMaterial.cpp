// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanMaterial.h"
#include "VulkanException.h"
#include "VulkanInstance.h"
#include "VulkanScreenQuad.h"
#include "VulkanDirectShadowMap.h"
#include "VulkanSamplerHolder.h"
#include "VulkanPassInfo.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "PostEffectManager.h"
#include "ShaderInfo.h"
#include "SceneManager.h"
#include "LightManager.h"
#include "ShadowMap.h"
#include "Water.h"
#include "VulkanWater.h"
#include "Entity.h"
#include "Engine.h"

#include <fstream>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <cmath>

namespace SVE
{
namespace
{
VkSamplerAddressMode getAddressMode(TextureAddressMode mode)
{
    static const std::map<TextureAddressMode, VkSamplerAddressMode> addressModeMap {
            { TextureAddressMode::Repeat,               VK_SAMPLER_ADDRESS_MODE_REPEAT },
            { TextureAddressMode::MirroredRepeat,       VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT },
            { TextureAddressMode::ClampToEdge,          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
            { TextureAddressMode::ClampToBorder,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER },
            { TextureAddressMode::MirrorClampToEdge,    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE },
    };

    return addressModeMap.at(mode);
}

VkBorderColor getBorderColor(TextureBorderColor color)
{
    switch (color)
    {
        case TextureBorderColor::TransparentBlack:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case TextureBorderColor::SolidBlack:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case TextureBorderColor::SolidWhite:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }

    throw VulkanException("Unsupported texture border color");
}

} // anon namespace

SVE::VulkanMaterial::VulkanMaterial(MaterialSettings materialSettings)
    : _materialSettings(std::move(materialSettings))
    , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _device(_vulkanInstance->getLogicalDevice())
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
    , _hasExternals(false)
{
    const auto& shaderManager = Engine::getInstance()->getShaderManager();

    if (_materialSettings.passType == CommandsType::ScreenQuadMRTPass)
    {
        _materialSettings.useMRT = true;
    }

    if (!_materialSettings.vertexShaderName.empty())
    {
        _vertexShader = shaderManager->getShader(_materialSettings.vertexShaderName)->getVulkanShaderInfo();
        _shaderList.push_back(_vertexShader);
    }
    if (!_materialSettings.geometryShaderName.empty())
    {
        _geometryShader = shaderManager->getShader(_materialSettings.geometryShaderName)->getVulkanShaderInfo();
        _shaderList.push_back(_geometryShader);
    }
    if (!_materialSettings.fragmentShaderName.empty())
    {
        _fragmentShader = shaderManager->getShader(_materialSettings.fragmentShaderName)->getVulkanShaderInfo();
        _shaderList.push_back(_fragmentShader);
    }

    createPipelineLayout();
    createPipeline();

    if (_materialSettings.isCubemap)
        createCubemapTextureImages();
    else
        createTextureImages();
    createTextureImageView();
    createTextureSampler();

    createUniformBuffers();
    createStorageBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

VulkanMaterial::~VulkanMaterial()
{
    deleteDescriptorSets();
    deleteDescriptorPool();
    deleteUniformBuffers();
    deleteStorageBuffers();

    deleteTextureSampler();
    deleteTextureImageView();
    deleteTextureImages();

    deletePipeline();
    deletePipelineLayout();
}

VkPipeline VulkanMaterial::getPipeline() const
{
    return _pipeline;
}

VkPipelineLayout VulkanMaterial::getPipelineLayout() const
{
    return _pipelineLayout;
}

void VulkanMaterial::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex, uint32_t materialIndex)
{
    if (_materialSettings.useInstancing && !isMainInstance(materialIndex) && isInstancesRendered())
        return;

    auto commandBuffer = _vulkanInstance->getCommandBuffer(bufferIndex);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

    auto descriptorSets = getDescriptorSets(materialIndex, imageIndex);
    vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            _pipelineLayout,
            0,
            descriptorSets.size(),
            descriptorSets.data(), 0, nullptr);
}

void VulkanMaterial::resetDescriptorSets()
{
   if (_hasExternals)
       updateDescriptorSets();
}

void VulkanMaterial::resetPipeline()
{
    deletePipeline();

    createPipeline();
}

std::vector<VkDescriptorSet> VulkanMaterial::getDescriptorSets(uint32_t materialIndex, size_t index) const
{
    std::vector<VkDescriptorSet> sets;
    sets.reserve(_shaderList.size());
    if (!_instanceData[materialIndex].vertexDescriptorSets.empty())
        sets.push_back(_instanceData[materialIndex].vertexDescriptorSets[index]);
    if (!_instanceData[materialIndex].geometryDescriptorSets.empty())
        sets.push_back(_instanceData[materialIndex].geometryDescriptorSets[index]);
    if (!_instanceData[materialIndex].fragmentDescriptorSets.empty())
        sets.push_back(_instanceData[materialIndex].fragmentDescriptorSets[index]);


    return sets;
}

uint32_t VulkanMaterial::getInstanceForEntity(const Entity* entity, uint32_t index)
{
    auto instanceIter = _entityInstanceMap.find(entity);
    if (instanceIter != _entityInstanceMap.end())
    {
        if (instanceIter->second.size() > index)
            return instanceIter->second[index];

        if (index > instanceIter->second.size())
            throw VulkanException("Incorrect entity material index");
        instanceIter->second.push_back(0);
    } else {
        _entityInstanceMap[entity] = std::vector<uint32_t>(1);
    }

    createUniformBuffers();
    //createStorageBuffers();
    createDescriptorPool();
    createDescriptorSets();
    _entityInstanceMap[entity][index] = _instanceData.size() - 1;
    return _instanceData.size() - 1;
}

void VulkanMaterial::deleteInstancesForEntity(const Entity* entity)
{
    auto instanceIter = _entityInstanceMap.find(entity);
    if (instanceIter == _entityInstanceMap.end())
    {
        return;
    }

    for (auto& index : instanceIter->second)
    {
        deleteDescriptorSets(_instanceData[index]);
        deleteDescriptorPool(_instanceData[index]);
        deleteUniformBuffers(_instanceData[index]);
        _instanceData[index] = {};
    }

    _entityInstanceMap.erase(instanceIter);
}

bool VulkanMaterial::isSkeletal() const
{
    if (!_vertexShader)
        return false;

    return _vertexShader->getShaderSettings().maxBonesSize > 0;
}

void VulkanMaterial::setUniformData(uint32_t materialIndex, const UniformData& uniformData)
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();
    auto imageIndex = _vulkanInstance->getCurrentImageIndex();

    if (_storageUpdated || !_materialSettings.useInstancing)
    {
        for (auto i = 0u; i < _shaderList.size(); i++)
        {
            size_t uniformSize = _shaderList[i]->getShaderUniformsSize();
            if (uniformSize == 0)
                continue;
            const auto& shaderSettings = _shaderList[i]->getShaderSettings();
            void* data = nullptr;
            vkMapMemory(_device, _instanceData[materialIndex].uniformBuffersMemory[swapchainSize * i + imageIndex], 0,
                        uniformSize, 0, &data);
            char* mappedUniformData = reinterpret_cast<char*>(data);
            for (const auto& r : shaderSettings.uniformList)
            {
                auto uniformBytes = getUniformDataByType(uniformData, r.uniformType);
                memcpy(mappedUniformData, uniformBytes.data(), uniformBytes.size());
                mappedUniformData += uniformBytes.size();
            }
            vkUnmapMemory(_device, _instanceData[materialIndex].uniformBuffersMemory[swapchainSize * i + imageIndex]);
        }
    }

    for (const auto& b : _vertexShader->getShaderSettings().bufferList)
    {
        updateStorageDataByUniforms(uniformData, _storageData, b);
    }

    if (_storageUpdated || !_materialSettings.useInstancing)
    {
        _currentInstanceCount = 0;
        _storageUpdated = false;
        _mainInstance = materialIndex;
        _instancesRendered = false;
    }

    ++_currentInstanceCount;
}

void VulkanMaterial::updateInstancedData()
{
    auto imageIndex = _vulkanInstance->getCurrentImageIndex();

    if (!_materialSettings.useInstancing)
        return;

    if (_storageBufferSize == 0 || _mainInstance == 0 || _storageUpdated)
        return;

    void* data = nullptr;
    vkMapMemory(_device, _storageBuffersMemory[imageIndex], 0, _storageBufferSize, 0, &data);
    char* mappedBufferData = reinterpret_cast<char*>(data);
    for (const auto& b : _vertexShader->getShaderSettings().bufferList)
    {
        auto storageBytes = getStorageDataByType(_storageData, b);
        if (!storageBytes.empty())
        {
            memcpy(mappedBufferData, storageBytes.data(), storageBytes.size());
            mappedBufferData += storageBytes.size();
        }
    }
    vkUnmapMemory(_device, _storageBuffersMemory[imageIndex]);
    _storageData.modelList.clear();
    _storageUpdated = true;
}

void VulkanMaterial::createPipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (auto * shader : _shaderList)
    {
        shaderStages.push_back(shader->createShaderStage());
    }

    // Triangle data setup
    auto bindingDescriptions = _vertexShader->getBindingDescription();
    auto attributeDescriptions = _vertexShader->getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport
    auto extent = _vulkanInstance->getExtent();
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo{};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.scissorCount = 1; // should be the same as viewport count
    viewportCreateInfo.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE; // clamp objects beyond near and far plane to the edges
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.lineWidth = 1.0f;

    switch (_materialSettings.cullFace)
    {
        case MaterialCullFace::BackFace:
            rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case MaterialCullFace::FrontFace:
            rasterizationCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case MaterialCullFace::None:
            rasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
            break;
    }

    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = _materialSettings.useDepthBias ? VK_TRUE : VK_FALSE; // for altering depth (used in shadow mapping)

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = _materialSettings.useMultisampling
                                                                ? _vulkanInstance->getMSAASamples()
                                                                : VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    // Depth and stencil
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    if (_materialSettings.useDepthTest)
    {
        depthStencilCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilCreateInfo.depthWriteEnable = _materialSettings.useDepthWrite ? VK_TRUE : VK_FALSE;
        depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    } else {
        depthStencilCreateInfo.depthTestEnable = VK_FALSE;
        depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
    }
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

    // Blending
    auto blendFactorToVulkan = [](BlendFactor blendFactor)
    {
        switch (blendFactor)
        {
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
        }

        assert(!"Bad blending value");
        return VK_BLEND_FACTOR_SRC_ALPHA;
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment[2] = {};
    for (auto i = 0; i < 2; ++i)
    {
        colorBlendAttachment[i].colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment[i].blendEnable = _materialSettings.useAlphaBlending ? VK_TRUE : VK_FALSE;
        colorBlendAttachment[i].srcColorBlendFactor = blendFactorToVulkan(_materialSettings.srcBlendFactor);
        colorBlendAttachment[i].dstColorBlendFactor = blendFactorToVulkan(_materialSettings.dstBlendFactor);
        colorBlendAttachment[i].colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment[i].srcAlphaBlendFactor = blendFactorToVulkan(_materialSettings.srcBlendFactor);
        colorBlendAttachment[i].dstAlphaBlendFactor = blendFactorToVulkan(_materialSettings.dstBlendFactor);;
        colorBlendAttachment[i].alphaBlendOp = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo blendingCreateInfo{};
    blendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendingCreateInfo.logicOpEnable = VK_FALSE;
    blendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    blendingCreateInfo.attachmentCount = _materialSettings.useMRT ? 2 : 1;
    blendingCreateInfo.pAttachments = colorBlendAttachment;
    blendingCreateInfo.blendConstants[0] = 1.0f;
    blendingCreateInfo.blendConstants[1] = 1.0f;
    blendingCreateInfo.blendConstants[2] = 1.0f;
    blendingCreateInfo.blendConstants[3] = 1.0f;

    std::vector<VkDynamicState> dynamicStateList = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicStateList.size();
    dynamicStateCreateInfo.pDynamicStates = dynamicStateList.data();

    // Finally create pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.pColorBlendState = &blendingCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = _pipelineLayout;
    pipelineCreateInfo.renderPass = _vulkanInstance->getPassInfo()->getPassData(_materialSettings.passType).renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no deriving from other pipeline
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_pipeline) !=
        VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Graphics Pipeline");
    }

    for (auto * shader : _shaderList)
    {
        shader->freeShaderModule();
    }
}

void VulkanMaterial::deletePipeline()
{
    vkDestroyPipeline(_device, _pipeline, nullptr);
}


void VulkanMaterial::createPipelineLayout()
{
    std::vector<VkDescriptorSetLayout> descriptorLayouts;

    for (auto* shader : _shaderList)
    {
        auto descriptorLayout = shader->getDescriptorSetLayout();
        if (descriptorLayout != VK_NULL_HANDLE)
            descriptorLayouts.push_back(descriptorLayout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = descriptorLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts.data();
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

void VulkanMaterial::createTextureImages()
{
    size_t imageCount = _materialSettings.textures.size();
    _mipLevels.resize(imageCount);
    _texturesData.resize(imageCount);
    _textureImages.resize(imageCount);
    _textureImageMemoryList.resize(imageCount);
    _textureNames.resize(imageCount);
    _textureImageViews.resize(imageCount);
    _textureSamplers.resize(imageCount);
    for (auto i = 0u; i < imageCount; i++)
    {
        _textureNames[i] = _materialSettings.textures[i].samplerName;

        if (_materialSettings.textures[i].textureType != TextureType::ImageFile)
        {
            _hasExternals = true;
            _texturesData[i].external = true;

            // TODO: For more generic cases, texture type should be string (or custom with additional string attr)
            _texturesData[i].type = _materialSettings.textures[i].textureType;
            if (!_materialSettings.textures[i].textureSubtype.empty())
                _texturesData[i].subtype = Engine::getInstance()->getPostEffectManager()->getEffectIndex(_materialSettings.textures[i].textureSubtype);
            continue;
        } else {
            _texturesData[i].external = false;
        }

        // Load image pixel data
        int texWidth, texHeight, texChannels;
        //stbi_set_flip_vertically_on_load(true);
        auto fileContent = Engine::getInstance()->getResourceManager()->loadFileContent(_materialSettings.textures[i].filename);
        stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(fileContent.data()), fileContent.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

        _mipLevels[i] = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        if (!pixels)
        {
            throw VulkanException("Can't load texture " + _materialSettings.textures[i].filename);
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
        _vulkanUtils.createImage(static_cast<uint32_t>(texWidth),
                                 static_cast<uint32_t>(texHeight),
                                 _mipLevels[i],
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 _textureImages[i],
                                 _textureImageMemoryList[i]);

        // Transition layout of the image to be optimal as a transfer destination
        _vulkanUtils.transitionImageLayout(
                _textureImages[i],
                VK_FORMAT_R8G8B8A8_UNORM,
                {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
                {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
                _mipLevels[i]);
        // Copy image data from buffer to image
        _vulkanUtils.copyBufferToImage(stagingBuffer,
                                       _textureImages[i],
                                       static_cast<uint32_t>(texWidth),
                                       static_cast<uint32_t>(texHeight));

        _vulkanUtils.generateMipmaps(_textureImages[i], VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, _mipLevels[i]);

        // Free temporary buffer
        vkDestroyBuffer(_device, stagingBuffer, nullptr);
        vkFreeMemory(_device, stagingBufferMemory, nullptr);
    }
}

void VulkanMaterial::createCubemapTextureImages()
{
    size_t imageCount = _materialSettings.textures.size();
    if (imageCount != 6)
        throw VulkanException("Incorrect cube map configuration (need 6 images)");

    _textureImages.resize(1);
    _textureImageMemoryList.resize(1);
    _textureNames.resize(1);
    _texturesData.resize(1);
    _textureImageViews.resize(1);
    _textureSamplers.resize(1);
    std::vector<stbi_uc*> pixelsData;
    VkDeviceSize imageSize = 0;
    int texWidth = 0, texHeight = 0, texChannels;
    for (auto i = 0u; i < imageCount; i++)
    {
        // Load image pixel data
        //stbi_set_flip_vertically_on_load(true);

        auto fileContent = Engine::getInstance()->getResourceManager()->loadFileContent(_materialSettings.textures[i].filename);
        stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const uint8_t*>(fileContent.data()), fileContent.size(), &texWidth, &texHeight, &texChannels,
                                    STBI_rgb_alpha);
        stbi_set_flip_vertically_on_load(false);
        imageSize += static_cast<VkDeviceSize>(texWidth * texHeight * 4);

        if (!pixels)
        {
            throw VulkanException("Can't load texture " + _materialSettings.textures[i].filename);
        }
        pixelsData.push_back(pixels);
    }
    auto singleImageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    //_mipLevels.push_back(static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1);
    _mipLevels.push_back(1);

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
    char* mappedData = reinterpret_cast<char*>(data);
    for (auto i = 0u; i < pixelsData.size(); i++)
    {
        memcpy(mappedData, pixelsData[i], singleImageSize);
        mappedData += singleImageSize;
    }
    vkUnmapMemory(_device, stagingBufferMemory);

    // Free pixel data
    for (auto i = 0u; i < imageCount; i++)
        stbi_image_free(pixelsData[i]);

    // Create texture image which will be used in shaders
    _vulkanUtils.createImage(static_cast<uint32_t>(texWidth),
                             static_cast<uint32_t>(texHeight),
                             _mipLevels[0],
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             _textureImages[0],
                             _textureImageMemoryList[0],
                             VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                             6);

    // Transition layout of the image to be optimal as a transfer destination
    _vulkanUtils.transitionImageLayout(
            _textureImages[0],
            VK_FORMAT_R8G8B8A8_UNORM,
            {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
            _mipLevels[0],
            VK_IMAGE_ASPECT_COLOR_BIT,
            6);


    // Copy image data from buffer to image
    auto commandBuffer = _vulkanUtils.beginRecordingCommands();
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    uint32_t offset = 0;
    for (auto i = 0u; i < 6; i++)
    {
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = i;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texWidth);
        bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texHeight);
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;

        bufferCopyRegions.push_back(bufferCopyRegion);

        // Increase offset into staging buffer for next level / face
        offset += texWidth * texHeight * 4;
    }

    vkCmdCopyBufferToImage(commandBuffer,
                           stagingBuffer,
                           _textureImages[0],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           bufferCopyRegions.size(),
                           bufferCopyRegions.data());

    _vulkanUtils.endRecordingAndSubmitCommands(commandBuffer);

    _vulkanUtils.transitionImageLayout(
         _textureImages[0],
         VK_FORMAT_R8G8B8A8_UNORM,
         {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
         {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
         _mipLevels[0],
         VK_IMAGE_ASPECT_COLOR_BIT,
         6);

    //_vulkanUtils.generateMipmaps(_textureImages[i], VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, _mipLevels[i]);

    // Free temporary buffer
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    _textureNames[0] = _materialSettings.textures[0].samplerName;
    _texturesData[0].external = false;

}

void VulkanMaterial::deleteTextureImages()
{
    for (auto i = 0u; i < _textureImages.size(); i++)
    {
        if (_texturesData[i].external)
            continue;

        vkDestroyImage(_device, _textureImages[i], nullptr);
        vkFreeMemory(_device, _textureImageMemoryList[i], nullptr);
    }
}

void VulkanMaterial::createTextureImageView()
{
    for (auto i = 0; i < _textureImages.size(); i++)
    {
        if (_texturesData[i].external)
            continue;

        _textureImageViews[i] = _vulkanUtils.createImageView(
                _textureImages[i],
                VK_FORMAT_R8G8B8A8_UNORM,
                _mipLevels[i],
                VK_IMAGE_ASPECT_COLOR_BIT,
                _materialSettings.isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
                _materialSettings.isCubemap ? 6 : 1);
    }
}

void VulkanMaterial::deleteTextureImageView()
{
    for (auto i = 0; i < _textureImageViews.size(); i++)
    {
        if (_texturesData[i].external)
            continue;
        vkDestroyImageView(_device, _textureImageViews[i], nullptr);
    }
}

void VulkanMaterial::createTextureSampler()
{
    for (auto i = 0; i < _textureImages.size(); i++)
    {
        if (_texturesData[i].external)
            continue;

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;

        auto addressMode = getAddressMode(_materialSettings.textures[i].textureAddressMode);
        samplerCreateInfo.addressModeU = addressMode;
        samplerCreateInfo.addressModeV = addressMode;
        samplerCreateInfo.addressModeW = addressMode;
        samplerCreateInfo.borderColor = getBorderColor(_materialSettings.textures[i].textureBorderColor);
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.maxAnisotropy = 16;
        //samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCreateInfo.minLod = 0;
        samplerCreateInfo.maxLod = _mipLevels[i];
        samplerCreateInfo.mipLodBias = 0;

        if (vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_textureSamplers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Can't create Vulkan texture sampler");
        }
    }
}

void VulkanMaterial::deleteTextureSampler()
{
    for (auto i = 0; i < _textureSamplers.size(); i++)
    {
        if (_texturesData[i].external)
            continue;

        vkDestroySampler(_device, _textureSamplers[i], nullptr);
    }
}

void VulkanMaterial::createUniformBuffers()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();
    size_t memoryBufferOffset = 0;
    PerInstanceData data {};

    auto createUniformBuffers = [this, &data, &swapchainSize, &memoryBufferOffset](std::vector<VkBuffer>& buffers,
                                                                                   const VulkanShaderInfo* shaderInfo)
    {
        if (!shaderInfo)
            return;

        VkDeviceSize bufferSize = shaderInfo->getShaderUniformsSize();
        data.uniformBuffersMemory.resize(data.uniformBuffersMemory.size() + swapchainSize);
        if (bufferSize == 0)
        {
            memoryBufferOffset += swapchainSize;
            return;
        }

        buffers.resize(swapchainSize);

        for (auto i = 0u; i < swapchainSize; i++)
        {
            _vulkanUtils.createBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    buffers[i],
                    data.uniformBuffersMemory[memoryBufferOffset + i]);
        }
        memoryBufferOffset += swapchainSize;
    };

    createUniformBuffers(data.vertexUniformBuffers, _vertexShader);
    createUniformBuffers(data.geometryUniformBuffer, _geometryShader);
    createUniformBuffers(data.fragmentUniformBuffer, _fragmentShader);

    _instanceData.push_back(data);
}

void VulkanMaterial::deleteUniformBuffers()
{
    for (auto& instance : _instanceData)
    {
        deleteUniformBuffers(instance);
    }
}

void VulkanMaterial::deleteUniformBuffers(PerInstanceData& instance)
{
    for (auto buffer : instance.vertexUniformBuffers)
    {
        vkDestroyBuffer(_device, buffer, nullptr);
    }
    for (auto buffer : instance.geometryUniformBuffer)
    {
        vkDestroyBuffer(_device, buffer, nullptr);
    }
    for (auto buffer : instance.fragmentUniformBuffer)
    {
        vkDestroyBuffer(_device, buffer, nullptr);
    }

    for (auto memory : instance.uniformBuffersMemory)
    {
        vkFreeMemory(_device, memory, nullptr);
    }
}

void VulkanMaterial::createStorageBuffers()
{
    if (!_storageBuffersMemory.empty())
        return;

    auto swapchainSize = _vulkanInstance->getSwapchainSize();

    _storageBufferSize = _vertexShader->getShaderStorageBuffersSize();
    if (_storageBufferSize == 0)
        _storageBufferSize = _materialSettings.instanceMaxCount;

    if (_storageBufferSize == 0)
        return;
    _storageBuffersMemory.resize(swapchainSize);
    _vertexStorageBuffers.resize(swapchainSize);

    for (auto i = 0u; i < swapchainSize; i++)
    {
        _vulkanUtils.createBuffer(
                _storageBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                _vertexStorageBuffers[i],
                _storageBuffersMemory[i]);
    }
}

void VulkanMaterial::deleteStorageBuffers()
{
    for (auto buffer : _vertexStorageBuffers)
    {
        vkDestroyBuffer(_device, buffer, nullptr);
    }

    for (auto memory : _storageBuffersMemory)
    {
        vkFreeMemory(_device, memory, nullptr);
    }
}


void VulkanMaterial::createDescriptorPool()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();

    auto& instance = _instanceData.back();

    std::vector<VkDescriptorPoolSize> poolSizes(3);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = instance.vertexUniformBuffers.size() + instance.fragmentUniformBuffer.size() + instance.geometryUniformBuffer.size();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapchainSize * _materialSettings.textures.size() * 2;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = _vertexStorageBuffers.size();
    for (auto& poolSize : poolSizes)
        if (poolSize.descriptorCount == 0)
            poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = _shaderList.size() * swapchainSize;

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &instance.descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan descriptor pool");
    }
}

void VulkanMaterial::deleteDescriptorPool()
{
    for (auto &instance : _instanceData)
    {
        deleteDescriptorPool(instance);
    }
}

void VulkanMaterial::deleteDescriptorPool(PerInstanceData& instance)
{
    vkDestroyDescriptorPool(_device, instance.descriptorPool, nullptr);
}

void VulkanMaterial::createDescriptorSets()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();
    auto makeDescriptorSet = [this, swapchainSize](const std::vector<VkBuffer>& shaderBuffers,
                                                   const std::vector<VkBuffer>* storageBuffers,
                                                   const VulkanShaderInfo* shaderInfo,
                                                   std::vector<VkDescriptorSet>& descriptorSets)
    {
        if (!shaderInfo)
            return;
        if (shaderInfo->getShaderSettings().samplerNamesList.empty() && shaderBuffers.empty())
            return;

        std::vector<VkDescriptorSetLayout> layouts(swapchainSize, shaderInfo->getDescriptorSetLayout());

        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _instanceData.back().descriptorPool;
        allocInfo.descriptorSetCount = swapchainSize;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapchainSize);
        auto result = vkAllocateDescriptorSets(_device, &allocInfo, descriptorSets.data());
        if (result != VK_SUCCESS)
        {
            throw VulkanException("Can't allocate Vulkan descriptor sets");
        }

        auto uniformSize = shaderInfo->getShaderUniformsSize();

        for (auto i = 0u; i < swapchainSize; i++)
        {
            VkDescriptorBufferInfo bufferInfo {};
            VkDescriptorBufferInfo storageBufferInfo {};
            if (!shaderBuffers.empty())
            {
                bufferInfo.buffer = shaderBuffers[i];
                bufferInfo.offset = 0;
                bufferInfo.range = uniformSize;
            }

            std::vector<VkDescriptorImageInfo> imageInfoList;
            for (const auto& samplerName : shaderInfo->getShaderSettings().samplerNamesList)
            {
                auto iter = std::find(_textureNames.cbegin(),
                                      _textureNames.cend(),
                                      samplerName);
                if (iter == _textureNames.end())
                {
                    throw VulkanException("Incorrect sampler name in material configuration");
                }
                auto index = std::distance(_textureNames.cbegin(), iter);

                if (!_texturesData[index].external)
                {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = _textureImageViews[index];
                    imageInfo.sampler = _textureSamplers[index];

                    imageInfoList.push_back(imageInfo);
                } else {

                    const auto& samplerInfoList =
                            _texturesData[index].type == TextureType::ScreenQuad && _texturesData[index].subtype > 0
                            ? _vulkanInstance->getSamplerHolder()->getPostEffectSamplerInfo(_texturesData[index].subtype)
                            : _vulkanInstance->getSamplerHolder()->getSamplerInfo(_texturesData[index].type);
                    if (!samplerInfoList.empty())
                    {
                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfo.imageView = samplerInfoList[i].imageView;
                        imageInfo.sampler = samplerInfoList[i].sampler;

                        imageInfoList.push_back(imageInfo);
                    }
                }
            }


            std::vector<VkWriteDescriptorSet> descriptorWrites;
            auto bindingIndex = 0u;
            // Add texture samplers
            if (!imageInfoList.empty())
            {
                VkWriteDescriptorSet imagesBuffer {};
                imagesBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                imagesBuffer.dstSet = descriptorSets[i];
                imagesBuffer.dstBinding = bindingIndex;
                imagesBuffer.dstArrayElement = 0;
                imagesBuffer.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imagesBuffer.descriptorCount = imageInfoList.size();
                imagesBuffer.pImageInfo = imageInfoList.data();
                descriptorWrites.push_back(imagesBuffer);
                bindingIndex += imageInfoList.size();
            }
            // Add uniforms
            if (!shaderBuffers.empty())
            {
                VkWriteDescriptorSet uniformsBuffer {};
                uniformsBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                uniformsBuffer.dstSet = descriptorSets[i];
                uniformsBuffer.dstBinding = bindingIndex;
                uniformsBuffer.dstArrayElement = 0;
                uniformsBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uniformsBuffer.descriptorCount = 1;
                uniformsBuffer.pBufferInfo = &bufferInfo;
                descriptorWrites.push_back(uniformsBuffer);
                bindingIndex++;
            }

            if (storageBuffers && !storageBuffers->empty() && _storageBufferSize > 0)
            {
                storageBufferInfo.buffer = storageBuffers->at(i);
                storageBufferInfo.offset = 0;
                storageBufferInfo.range = _storageBufferSize;

                VkWriteDescriptorSet storageBuffer {};
                storageBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                storageBuffer.dstSet = descriptorSets[i];
                storageBuffer.dstBinding = bindingIndex;
                storageBuffer.dstArrayElement = 0;
                storageBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                storageBuffer.descriptorCount = 1;
                storageBuffer.pBufferInfo = &storageBufferInfo;
                descriptorWrites.push_back(storageBuffer);
            }

            vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
    };

    makeDescriptorSet(_instanceData.back().vertexUniformBuffers, &_vertexStorageBuffers,  _vertexShader, _instanceData.back().vertexDescriptorSets);
    makeDescriptorSet(_instanceData.back().geometryUniformBuffer, nullptr, _geometryShader, _instanceData.back().geometryDescriptorSets);
    makeDescriptorSet(_instanceData.back().fragmentUniformBuffer, nullptr, _fragmentShader, _instanceData.back().fragmentDescriptorSets);
}

void VulkanMaterial::deleteDescriptorSets()
{

}

void VulkanMaterial::deleteDescriptorSets(VulkanMaterial::PerInstanceData& instance)
{

}

void VulkanMaterial::updateDescriptorSets()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();
    auto makeDescriptorSet = [this, swapchainSize](const std::vector<VkBuffer>& shaderBuffers,
                                                   const std::vector<VkBuffer>* storageBuffers,
                                                   const VulkanShaderInfo* shaderInfo,
                                                   std::vector<VkDescriptorSet>& descriptorSets)
    {
        if (!shaderInfo)
            return;
        if (shaderInfo->getShaderSettings().samplerNamesList.empty() && shaderBuffers.empty())
            return;

        auto uniformSize = shaderInfo->getShaderUniformsSize();

        for (auto i = 0u; i < swapchainSize; i++)
        {
            updateDescriptorSet(
                    i,
                    shaderBuffers.empty() ? nullptr : &shaderBuffers[i],
                    storageBuffers && !storageBuffers->empty() ? &storageBuffers->at(i) : nullptr,
                    uniformSize,
                    storageBuffers ? _storageBufferSize : 0,
                    shaderInfo,
                    descriptorSets[i]);
        }
    };

    makeDescriptorSet(_instanceData.back().vertexUniformBuffers, &_vertexStorageBuffers, _vertexShader, _instanceData.back().vertexDescriptorSets);
    makeDescriptorSet(_instanceData.back().geometryUniformBuffer, nullptr, _geometryShader, _instanceData.back().geometryDescriptorSets);
    makeDescriptorSet(_instanceData.back().fragmentUniformBuffer, nullptr, _fragmentShader, _instanceData.back().fragmentDescriptorSets);
}

void VulkanMaterial::updateDescriptorSet(
        uint32_t imageIndex,
        const VkBuffer* shaderBuffer,
        const VkBuffer* storageBuffer,
        const size_t uniformSize,
        const size_t storageSize,
        const VulkanShaderInfo* shaderInfo,
        VkDescriptorSet descriptorSet)
{
    VkDescriptorBufferInfo bufferInfo {};
    VkDescriptorBufferInfo storageBufferInfo {};
    if (shaderBuffer)
    {
        bufferInfo.buffer = *shaderBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = uniformSize;
    }
    if (storageBuffer)
    {
        storageBufferInfo.buffer = *storageBuffer;
        storageBufferInfo.offset = 0;
        storageBufferInfo.range = storageSize;
    }

    std::vector<VkDescriptorImageInfo> imageInfoList;
    for (const auto& samplerName : shaderInfo->getShaderSettings().samplerNamesList)
    {
        auto iter = std::find(_textureNames.cbegin(),
                              _textureNames.cend(),
                              samplerName);
        if (iter == _textureNames.end())
        {
            throw VulkanException("Incorrect sampler name in material configuration");
        }
        auto index = std::distance(_textureNames.cbegin(), iter);

        if (!_texturesData[index].external)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = _textureImageViews[index];
            imageInfo.sampler = _textureSamplers[index];

            imageInfoList.push_back(imageInfo);
        } else {
            const auto& samplerInfoList =
                    _texturesData[index].type == TextureType::ScreenQuad && _texturesData[index].subtype > 0
                    ? _vulkanInstance->getSamplerHolder()->getPostEffectSamplerInfo(_texturesData[index].subtype)
                    : _vulkanInstance->getSamplerHolder()->getSamplerInfo(_texturesData[index].type);
            if (!samplerInfoList.empty() && samplerInfoList[imageIndex].imageView != VK_NULL_HANDLE)
            {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = samplerInfoList[imageIndex].imageView;
                imageInfo.sampler = samplerInfoList[imageIndex].sampler;

                imageInfoList.push_back(imageInfo);
            }
        }
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    auto bindingIndex = 0u;
    // Add texture samplers
    if (!imageInfoList.empty())
    {
        VkWriteDescriptorSet imagesBuffer {};
        imagesBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imagesBuffer.dstSet = descriptorSet;
        imagesBuffer.dstBinding = bindingIndex;
        imagesBuffer.dstArrayElement = 0;
        imagesBuffer.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imagesBuffer.descriptorCount = imageInfoList.size();
        imagesBuffer.pImageInfo = imageInfoList.data();
        descriptorWrites.push_back(imagesBuffer);
        bindingIndex += imageInfoList.size();
    }
    // Add uniforms
    if (shaderBuffer)
    {
        VkWriteDescriptorSet uniformsBuffer {};
        uniformsBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformsBuffer.dstSet = descriptorSet;
        uniformsBuffer.dstBinding = bindingIndex;
        uniformsBuffer.dstArrayElement = 0;
        uniformsBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformsBuffer.descriptorCount = 1;
        uniformsBuffer.pBufferInfo = &bufferInfo;
        descriptorWrites.push_back(uniformsBuffer);
        ++bindingIndex;
    }

    if (storageBuffer)
    {
        VkWriteDescriptorSet storageBuffer {};
        storageBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        storageBuffer.dstSet = descriptorSet;
        storageBuffer.dstBinding = bindingIndex;
        storageBuffer.dstArrayElement = 0;
        storageBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        storageBuffer.descriptorCount = 1;
        storageBuffer.pBufferInfo = &storageBufferInfo;
        descriptorWrites.push_back(storageBuffer);
    }

    vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

// TODO: Refactor this
glm::ivec2 VulkanMaterial::getSpritesheetSize() const
{
    for (auto& texture :_materialSettings.textures)
    {
        if (texture.spritesheetSize.x > 0)
            return texture.spritesheetSize;
    }
    return glm::ivec2();
}

const MaterialSettings &VulkanMaterial::getSettings() const
{
    return _materialSettings;
}

bool VulkanMaterial::isMainInstance(uint32_t materialIndex) const
{
    return _materialSettings.useInstancing && materialIndex == _mainInstance;
}

void VulkanMaterial::setMainInstance(uint32_t materialIndex)
{
    _mainInstance = materialIndex;
}

bool VulkanMaterial::isInstancesRendered() const
{
    return _instancesRendered;
}

void VulkanMaterial::setInstancedRendered()
{
    _instancesRendered = true;
}

uint32_t VulkanMaterial::getInstanceCount() const
{
    return _currentInstanceCount;//_instanceData.size() - 1;
}


} // namespace SVE