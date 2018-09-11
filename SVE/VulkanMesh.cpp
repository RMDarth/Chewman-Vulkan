// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Libs.h"
#include "VulkanMesh.h"
#include "Engine.h"
#include "VulkanMaterial.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"

namespace SVE
{

VulkanMesh::VulkanMesh(MeshSettings meshSettings)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
    , _meshSettings(std::move(meshSettings))
{
    createGeometryBuffers();
}

VulkanMesh::~VulkanMesh()
{
    deleteGeometryBuffers();
}

void VulkanMesh::applyDrawingCommands(uint32_t bufferIndex, VulkanMaterial* material, uint32_t materialIndex)
{
    auto commandBuffer = _vulkanInstance->getCommandBuffersList()[bufferIndex];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->getPipeline());

    std::vector<VkDeviceSize> offsets(_vertexBufferList.size());
    vkCmdBindVertexBuffers(commandBuffer, 0, _vertexBufferList.size(), _vertexBufferList.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    auto descriptorSets = material->getDescriptorSets(materialIndex, bufferIndex);
    vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            material->getPipelineLayout(),
            0,
            descriptorSets.size(),
            descriptorSets.data(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, _meshSettings.indexData.size(), 1, 0, 0, 0);
}

void VulkanMesh::createGeometryBuffers()
{
    // TODO: This can be optimized to use single buffer
    if (!_meshSettings.vertexPosData.empty())
    {
        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexPosData, 
                              _vertexBufferList.back(), 
                              _vertexBufferMemoryList.back(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    if (!_meshSettings.vertexColorData.empty())
    {
        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexColorData,
                              _vertexBufferList.back(),
                              _vertexBufferMemoryList.back(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    if (!_meshSettings.vertexTexData.empty())
    {
        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexTexData,
                              _vertexBufferList.back(),
                              _vertexBufferMemoryList.back(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    
    createOptimizedBuffer(_meshSettings.indexData, _indexBuffer, _indexBufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void VulkanMesh::deleteGeometryBuffers()
{
    vkDestroyBuffer(_vulkanInstance->getLogicalDevice(), _indexBuffer, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), _indexBufferMemory, nullptr);

    if (!_meshSettings.vertexPosData.empty())
    {
        vkDestroyBuffer(_vulkanInstance->getLogicalDevice(), _vertexBufferList[0], nullptr);
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), _vertexBufferMemoryList[0], nullptr);
    }
}

// This method will create fast GPU-local buffer (using transitional temporary CPU visible buffer)
template <typename T>
void VulkanMesh::createOptimizedBuffer(
        const std::vector<T>& bufferData,
        VkBuffer &buffer,
        VkDeviceMemory &deviceMemory,
        VkBufferUsageFlags usage)
{
    VkDeviceSize bufferSize = sizeof(bufferData[0]) * bufferData.size();

    // create temporary slow CPU visible memory
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _vulkanUtils.createBuffer(bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              stagingBuffer,
                              stagingBufferMemory);

    // Fill temporary buffer with vertex/index data
    void* data;
    vkMapMemory(_vulkanInstance->getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData.data(), (size_t) bufferSize);
    vkUnmapMemory(_vulkanInstance->getLogicalDevice(), stagingBufferMemory);

    // create fast GPU-local buffer for data
    _vulkanUtils.createBuffer(bufferSize,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              buffer,
                              deviceMemory);

    // copy slow CPU-visible buffer to fast GPU-local
    _vulkanUtils.copyBuffer(stagingBuffer, buffer, bufferSize);

    // destroy CPU-visible buffer
    vkDestroyBuffer(_vulkanInstance->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(_vulkanInstance->getLogicalDevice(), stagingBufferMemory, nullptr);
}

} // namespace SVE