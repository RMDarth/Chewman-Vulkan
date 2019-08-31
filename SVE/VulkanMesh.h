// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "MeshSettings.h"
#include <vulkan/vulkan.h>
#include <memory>

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
            VkDeviceMemory &deviceMemory,
            VkBufferUsageFlags usage);

private:
    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;

    MeshSettings _meshSettings;

    std::vector<VkBuffer> _vertexBufferList;
    std::vector<VkDeviceMemory> _vertexBufferMemoryList;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;
};

} // namespace SVE