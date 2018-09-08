// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Libs.h"
#include "VulkanMesh.h"
#include "Engine.h"
#include "MaterialManager.h"
#include "VulkanMaterial.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanException.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

VulkanMesh::VulkanMesh(MeshSettings meshSettings)
    : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
    , _vulkanUtils(_vulkanInstance->getVulkanUtils())
    , _meshSettings(std::move(meshSettings))
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(_meshSettings.materialName))
{
    createGraphicsPipeline();
    createGeometryBuffers();
    createCommandBuffers();
    createSemaphores();
}

VulkanMesh::~VulkanMesh()
{
    deleteSemaphores();
    deleteCommandBuffers();
    deleteGeometryBuffers();
    deleteGraphicsPipeline();
}

void VulkanMesh::createGraphicsPipeline()
{
    _pipeline = Engine::getInstance()->getVulkanInstance()->createPipeline(*this);
}

void VulkanMesh::deleteGraphicsPipeline()
{
    vkDestroyPipeline(Engine::getInstance()->getVulkanInstance()->getLogicalDevice(), _pipeline, nullptr);
}


VulkanMaterial* VulkanMesh::getMaterial() const
{
    return _material->getVulkanMaterial();
}

std::vector<VkVertexInputBindingDescription> VulkanMesh::getBindingDescription() const
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(3);

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(glm::vec3);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance

    bindingDescriptions[1].binding = 1;
    bindingDescriptions[1].stride = sizeof(glm::vec3);
    bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance

    bindingDescriptions[2].binding = 2;
    bindingDescriptions[2].stride = sizeof(glm::vec2);
    bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VulkanMesh::getAttributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    //attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 1;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    //attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 2;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    //attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

void VulkanMesh::updateMatrices()
{
    updateUniformBuffer(_vulkanInstance->getCurrentImageIndex());
}

VkSubmitInfo VulkanMesh::createSubmitInfo()
{
    auto currentImageIndex = _vulkanInstance->getCurrentImageIndex();

    // submit buffer to queue
    static VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = _vulkanInstance->getImageAvailableSemaphore();
    submitInfo.pWaitDstStageMask = waitStages; // should be same size as wait semaphores
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[currentImageIndex]; // command buffer to submit (should be the one with current swap chain image attachment)
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_renderFinishedSemaphore[currentImageIndex];

    return submitInfo;
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

void VulkanMesh::createCommandBuffers()
{
    _commandBuffers.resize(_vulkanInstance->getSwapchainSize());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = _vulkanInstance->getCommandPool();
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) _commandBuffers.size();

    if (vkAllocateCommandBuffers(_vulkanInstance->getLogicalDevice(), &commandBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Can't create Vulkan Command Buffers");
    }

    for (size_t i = 0; i < _commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        // start recording
        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording Vulkan command buffer");
        }

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = _vulkanInstance->getRenderPass();
        renderPassBeginInfo.framebuffer = _vulkanInstance->getFramebuffer(i);
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = _vulkanInstance->getExtent();
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

        std::vector<VkDeviceSize> offsets(_vertexBufferList.size());
        vkCmdBindVertexBuffers(_commandBuffers[i], 0, _vertexBufferList.size(), _vertexBufferList.data(), offsets.data());
        vkCmdBindIndexBuffer(_commandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        auto descriptorSet = _material->getVulkanMaterial()->getDescriptorSet(i);
        vkCmdBindDescriptorSets(
                _commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                _material->getVulkanMaterial()->getPipelineLayout(),
                0,
                1,
                &descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(_commandBuffers[i], _meshSettings.indexData.size(), 1, 0, 0, 0);
        vkCmdEndRenderPass(_commandBuffers[i]);

        // finish recording
        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
        {
            throw VulkanException("Failed to record Vulkan command buffer");
        }
    }
}

void VulkanMesh::deleteCommandBuffers()
{

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

void VulkanMesh::createSemaphores()
{
    _renderFinishedSemaphore.resize(_vulkanInstance->getSwapchainSize());

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        if (vkCreateSemaphore(
                _vulkanInstance->getLogicalDevice(),
                &semaphoreCreateInfo,
                nullptr,
                &_renderFinishedSemaphore[i]) != VK_SUCCESS)
        {
            throw VulkanException("Failed to create Vulkan semaphore");
        }
    }
}

void VulkanMesh::deleteSemaphores()
{
    for (auto i = 0u; i < _vulkanInstance->getSwapchainSize(); i++)
    {
        vkDestroySemaphore(_vulkanInstance->getLogicalDevice(), _renderFinishedSemaphore[i], nullptr);
    }
}

void VulkanMesh::updateUniformBuffer(uint32_t currentImage)
{
    // TODO: Refactor this method, time work shouldn't be here
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    auto extent = _vulkanInstance->getExtent();

    VulkanMaterial::MatricesUBO matrices {};
    matrices.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.model = glm::rotate(matrices.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    matrices.view = glm::lookAt(glm::vec3(200.0f, 200.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.projection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 1000.0f);
    matrices.projection[1][1] *= -1; // because OpenGL use inverted Y axis

    _material->getVulkanMaterial()->updateMatrices(matrices);
}

} // namespace SVE