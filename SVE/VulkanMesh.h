// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "MeshSettings.h"
#include "VulkanHeaders.h"
#include <memory>
#include <vk_mem_alloc.h>

namespace SVE
{
class VulkanMaterial;
class VulkanInstance;
class VulkanUtils;

class VulkanMesh
{
public:
    explicit VulkanMesh(MeshSettings meshSettings);
    ~VulkanMesh();

    void updateMesh(MeshSettings meshSettings);

    void applyDrawingCommands(uint32_t bufferIndex, uint32_t instanceCount = 1);

    const MeshSettings& getMeshSettings() const;

private:
    void createGeometryBuffers();
    void deleteGeometryBuffers();

private:
    template <typename T>
    void createOptimizedBuffer(
            const std::vector<T>& data,
            VkBuffer &buffer,
            VmaAllocation& allocation,
            VkBufferUsageFlags usage);

private:
    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;

    MeshSettings _meshSettings;

    std::vector<VkBuffer> _vertexBufferList;
    std::vector<VmaAllocation> _vertexBufferMemoryList;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VmaAllocation _indexBufferMemory = VK_NULL_HANDLE;
};

} // namespace SVE