// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "ComputeSettings.h"
#include "VulkanHeaders.h"
#include <vulkan/vk_mem_alloc.h>

#include <vector>

namespace SVE
{

class VulkanUtils;
class VulkanShaderInfo;
class VulkanInstance;

class VulkanComputeEntity
{
public:
    explicit VulkanComputeEntity(ComputeSettings computeSettings);
    ~VulkanComputeEntity();

    void reallocateCommandBuffers();

    VkBuffer getComputeBuffer() const;

    void setUniformData(const UniformData& uniformData) const;

    void applyComputeCommands() const;
    static void startComputeStep();
    static void finishComputeStep();

private:
    void createPipelineLayout();
    void deletePipelineLayout();

    void createPipeline();
    void deletePipeline();

    void createBufferResources();
    void deleteBufferResources();

    void createUniformAndStorageBuffers();
    void deleteUniformAndStorageBuffers();

    void createDescriptorPool();
    void deleteDescriptorPool();
    void createDescriptorSets();
    void deleteDescriptorSets();

private:
    ComputeSettings _computeSettings;

    VulkanInstance* _vulkanInstance;
    VkDevice _device;
    const VulkanUtils& _vulkanUtils;

    VulkanShaderInfo* _computeShader = nullptr;

    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline = VK_NULL_HANDLE;

    VmaAllocation _bufferMemory;
    VkBuffer _buffer;
    VkBufferView _bufferView;

    std::vector<VmaAllocation> _uniformBuffersMemory;
    std::vector<VkBuffer> _uniformBuffers;

    std::vector<VmaAllocation> _storageBuffersMemory;
    std::vector<VkBuffer> _storageBuffers;

    std::vector<VkDescriptorSet> _descriptorSets;
    VkDescriptorPool _descriptorPool;

    VkCommandBuffer _commandBuffer;
    bool _computeShaderNotSupported = false;

};

} // namespace SVE