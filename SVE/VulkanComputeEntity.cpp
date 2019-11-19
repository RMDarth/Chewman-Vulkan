// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanComputeEntity.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include "ShaderManager.h"

namespace SVE
{

VulkanComputeEntity::VulkanComputeEntity(ComputeSettings computeSettings)
        : _computeSettings(std::move(computeSettings))
        , _vulkanInstance(Engine::getInstance()->getVulkanInstance())
        , _device(_vulkanInstance->getLogicalDevice())
        , _vulkanUtils(_vulkanInstance->getVulkanUtils())
{
    const auto& shaderManager = Engine::getInstance()->getShaderManager();
    _computeShader = shaderManager->getShader(_computeSettings.computeShaderName)->getVulkanShaderInfo();

    createPipelineLayout();
    createPipeline();

    createBufferResources();

    createUniformAndStorageBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

VulkanComputeEntity::~VulkanComputeEntity()
{
    deleteDescriptorSets();
    deleteDescriptorPool();
    deleteUniformAndStorageBuffers();

    deleteBufferResources();

    deletePipeline();
    deletePipelineLayout();
}

VkBuffer VulkanComputeEntity::getComputeBuffer() const
{
    return _buffer;
}

void VulkanComputeEntity::setUniformData(const UniformData &uniformData) const
{
    auto imageIndex = _vulkanInstance->getCurrentImageIndex();

    size_t uniformSize = _computeShader->getShaderUniformsSize();
    if (uniformSize == 0)
        return;

    const auto& shaderSettings = _computeShader->getShaderSettings();
    char* data = nullptr;
    vmaMapMemory(_vulkanInstance->getAllocator(),  _uniformBuffersMemory[imageIndex], (void**)&data);
    for (const auto& r : shaderSettings.uniformList)
    {
        auto uniformBytes = getUniformDataByType(uniformData, r.uniformType);
        memcpy(data, uniformBytes.data(), uniformBytes.size());
        data += uniformBytes.size();
    }
    vmaUnmapMemory(_vulkanInstance->getAllocator(), _uniformBuffersMemory[imageIndex]);
}

void VulkanComputeEntity::reallocateCommandBuffers()
{
    // TODO: have different command for each in-flight frame
    _commandBuffer = _vulkanInstance->createCommandBuffer(BUFFER_INDEX_COMPUTE_PARTICLES);
}

void VulkanComputeEntity::applyComputeCommands() const
{
    if (_computeShaderNotSupported)
        return;

    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
    vkCmdBindDescriptorSets(
            _commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            _pipelineLayout,
            0,
            1,
            &_descriptorSets[_vulkanInstance->getCurrentImageIndex()], 0, nullptr);

    vkCmdDispatch(_commandBuffer, _computeSettings.elementsCount / 32, 1, 1);
}

void VulkanComputeEntity::createPipelineLayout()
{
    auto descriptorLayout = _computeShader->getDescriptorSetLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorLayout;
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

void VulkanComputeEntity::deletePipelineLayout()
{
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}

void VulkanComputeEntity::createPipeline()
{
    VkPipelineShaderStageCreateInfo shaderStage;
    shaderStage = _computeShader->createShaderStage();

    // Finally create pipeline
    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStage;
    pipelineCreateInfo.layout = _pipelineLayout;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no deriving from other pipeline
    pipelineCreateInfo.basePipelineIndex = -1;

    if (!_vulkanInstance->getEngineSettings().particlesEnabled)
    {
        _computeShaderNotSupported = true;
    }
    else
    {
        if (vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_pipeline) !=
            VK_SUCCESS)
        {
            _computeShaderNotSupported = true;
            _vulkanInstance->disableParticles();
            //throw VulkanException("Can't create Vulkan Compute Pipeline for shader " + _computeSettings.computeShaderName, result);
        }
    }

    _computeShader->freeShaderModule();
}

void VulkanComputeEntity::deletePipeline()
{
    if (_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(_device, _pipeline, nullptr);
}

void VulkanComputeEntity::createBufferResources()
{
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    _vulkanUtils.createOptimizedBuffer(_computeSettings.data.data(), _computeSettings.data.size(), _buffer, _bufferMemory, flags);

    VkBufferViewCreateInfo bufferViewCreateInfo{};
    bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bufferViewCreateInfo.buffer = _buffer;
    bufferViewCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    bufferViewCreateInfo.offset = 0;
    bufferViewCreateInfo.range = VK_WHOLE_SIZE;

    VkResult result = vkCreateBufferView(_device, &bufferViewCreateInfo, nullptr, &_bufferView);
    if( result != VK_SUCCESS )
    {
        throw VulkanException("Can't create Vulkan buffer view");
    }
}

void VulkanComputeEntity::deleteBufferResources()
{
    vmaDestroyBuffer(_vulkanInstance->getAllocator(), _buffer, _bufferMemory);
    vkDestroyBufferView(_device, _bufferView, nullptr);
}

void VulkanComputeEntity::createUniformAndStorageBuffers()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();

    VkDeviceSize uniformBufferSize = _computeShader->getShaderUniformsSize();
    VkDeviceSize storageBufferSize = _computeShader->getShaderStorageBuffersSize();
    _uniformBuffersMemory.resize(swapchainSize);
    _uniformBuffers.resize(swapchainSize);
    _storageBuffersMemory.resize(swapchainSize);
    _storageBuffers.resize(swapchainSize);

    for (auto i = 0u; i < swapchainSize; i++)
    {
        _vulkanUtils.createBuffer(
                uniformBufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _uniformBuffers[i],
                _uniformBuffersMemory[i]);

        _vulkanUtils.createBuffer(
                storageBufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                _storageBuffers[i],
                _storageBuffersMemory[i]);
    }


}

void VulkanComputeEntity::deleteUniformAndStorageBuffers()
{
    for (auto i = 0; i < _uniformBuffers.size(); ++i)
    {
        vmaDestroyBuffer(_vulkanInstance->getAllocator(), _uniformBuffers[i], _uniformBuffersMemory[i]);
    }
    for (auto i = 0; i < _storageBuffers.size(); ++i)
    {
        vmaDestroyBuffer(_vulkanInstance->getAllocator(), _storageBuffers[i], _storageBuffersMemory[i]);
    }
}

void VulkanComputeEntity::createDescriptorPool()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();

    std::vector<VkDescriptorPoolSize> poolSizes(3);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = _uniformBuffers.size();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = _storageBuffers.size();
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    poolSizes[2].descriptorCount = swapchainSize;
    for (auto& poolSize : poolSizes)
        if (poolSize.descriptorCount == 0)
            poolSize.descriptorCount = 1;

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

void VulkanComputeEntity::deleteDescriptorPool()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanComputeEntity::createDescriptorSets()
{
    auto swapchainSize = _vulkanInstance->getSwapchainSize();

    if (_computeShader->getShaderSettings().samplerNamesList.empty() && _uniformBuffers.empty())
        return;

    std::vector<VkDescriptorSetLayout> layouts(swapchainSize, _computeShader->getDescriptorSetLayout());

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = swapchainSize;
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(swapchainSize);
    auto result = vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        throw VulkanException("Can't allocate Vulkan descriptor sets");
    }

    auto uniformSize = _computeShader->getShaderUniformsSize();
    auto storageSize = _computeShader->getShaderStorageBuffersSize();

    for (auto i = 0u; i < swapchainSize; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo {};
        VkDescriptorBufferInfo storageBufferInfo {};
        if (!_uniformBuffers.empty())
        {
            uniformBufferInfo.buffer = _uniformBuffers[i];
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = uniformSize;
        }
        if (!_storageBuffers.empty())
        {
            storageBufferInfo.buffer = _storageBuffers[i];
            storageBufferInfo.offset = 0;
            storageBufferInfo.range = storageSize;
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        auto bindingIndex = 0u;
        // Add texture samplers
        if (_buffer != VK_NULL_HANDLE)
        {
            VkWriteDescriptorSet dataBuffer {};
            dataBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            dataBuffer.dstSet = _descriptorSets[i];
            dataBuffer.dstBinding = bindingIndex;
            dataBuffer.dstArrayElement = 0;
            dataBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            dataBuffer.descriptorCount = 1;
            dataBuffer.pTexelBufferView = &_bufferView;
            descriptorWrites.push_back(dataBuffer);
            bindingIndex ++;
        }
        // Add uniforms
        if (!_uniformBuffers.empty())
        {
            VkWriteDescriptorSet uniformsBuffer {};
            uniformsBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uniformsBuffer.dstSet = _descriptorSets[i];
            uniformsBuffer.dstBinding = bindingIndex;
            uniformsBuffer.dstArrayElement = 0;
            uniformsBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uniformsBuffer.descriptorCount = 1;
            uniformsBuffer.pBufferInfo = &uniformBufferInfo;
            descriptorWrites.push_back(uniformsBuffer);
            bindingIndex++;
        }

        if (!_storageBuffers.empty())
        {
            VkWriteDescriptorSet storageBuffer {};
            storageBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            storageBuffer.dstSet = _descriptorSets[i];
            storageBuffer.dstBinding = bindingIndex;
            storageBuffer.dstArrayElement = 0;
            storageBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            storageBuffer.descriptorCount = 1;
            storageBuffer.pBufferInfo = &storageBufferInfo;
            descriptorWrites.push_back(storageBuffer);
        }

        vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanComputeEntity::deleteDescriptorSets()
{

}

void VulkanComputeEntity::finishComputeStep()
{
    auto commandBuffer = Engine::getInstance()->getVulkanInstance()->getCommandBuffer(BUFFER_INDEX_COMPUTE_PARTICLES);

    // finish recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw VulkanException("Failed to record Vulkan command buffer");
    }

}

void VulkanComputeEntity::startComputeStep()
{
    auto commandBuffer = Engine::getInstance()->getVulkanInstance()->createCommandBuffer(BUFFER_INDEX_COMPUTE_PARTICLES);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VulkanException("Failed to begin recording Vulkan command buffer");
    }
}

} // namespace SVE