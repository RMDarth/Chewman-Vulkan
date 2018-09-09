// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "MeshSettings.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace SVE
{
class Material;
class VulkanMaterial;
class VulkanInstance;
class VulkanUtils;

class VulkanMesh
{
public:
    explicit VulkanMesh(MeshSettings meshSettings);
    ~VulkanMesh();

    VulkanMaterial* getMaterial() const;

    std::vector<VkVertexInputBindingDescription> getBindingDescription() const;
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

    VkSubmitInfo createSubmitInfo();

private:
    void createGraphicsPipeline();
    void deleteGraphicsPipeline();

    void createGeometryBuffers();
    void deleteGeometryBuffers();

    void createCommandBuffers();
    void deleteCommandBuffers();

    void createSemaphores();
    void deleteSemaphores();

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
    std::shared_ptr<Material> _material;

    VkPipeline _pipeline;

    std::vector<VkBuffer> _vertexBufferList;
    std::vector<VkDeviceMemory> _vertexBufferMemoryList;
    VkBuffer _indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> _commandBuffers;

    std::vector<VkSemaphore> _renderFinishedSemaphore;
};

} // namespace SVE