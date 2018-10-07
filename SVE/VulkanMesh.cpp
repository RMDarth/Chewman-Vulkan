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

void VulkanMesh::updateMesh(MeshSettings meshSettings)
{
    deleteGeometryBuffers();
    _meshSettings = std::move(meshSettings);
    createGeometryBuffers();
}

void VulkanMesh::applyDrawingCommands(uint32_t bufferIndex)
{
    auto commandBuffer = _vulkanInstance->getCommandBuffer(bufferIndex);

    std::vector<VkDeviceSize> offsets(_vertexBufferList.size());
    vkCmdBindVertexBuffers(commandBuffer, 0, _vertexBufferList.size(), _vertexBufferList.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, _meshSettings.indexData.size(), 1, 0, 0, 0);
}

const MeshSettings& VulkanMesh::getMeshSettings() const
{
    return _meshSettings;
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

    if (!_meshSettings.vertexNormalData.empty())
    {
        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexNormalData,
                              _vertexBufferList.back(),
                              _vertexBufferMemoryList.back(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    if (_meshSettings.boneNum > 0)
    {
        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexBoneWeightData,
                              _vertexBufferList.back(),
                              _vertexBufferMemoryList.back(),
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        _vertexBufferList.push_back(VK_NULL_HANDLE);
        _vertexBufferMemoryList.push_back(VK_NULL_HANDLE);
        createOptimizedBuffer(_meshSettings.vertexBoneIndexData,
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

    for (auto& buffer : _vertexBufferList)
    {
        vkDestroyBuffer(_vulkanInstance->getLogicalDevice(), buffer, nullptr);
    }
    for (auto& memory : _vertexBufferMemoryList)
    {
        vkFreeMemory(_vulkanInstance->getLogicalDevice(), memory, nullptr);
    }

    _vertexBufferList.clear();
    _vertexBufferMemoryList.clear();

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