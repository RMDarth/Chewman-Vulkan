// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Libs.h"
#include "MaterialSettings.h"

namespace SVE
{
class VulkanUtils;

class VulkanMaterial
{
public:
    // TODO: Refactor this struct (probably each matrix should be used individually depending on material configuration)
    // TODO: model matrix should be moved to mesh, view and projection - to camera
    // TODO: material though should get all those matrices and bind them to configured uniforms
    struct MatricesUBO
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    VulkanMaterial(MaterialSettings materialSettings);
    ~VulkanMaterial();

    std::vector<VkPipelineShaderStageCreateInfo> createShaderStages() const;
    void freeShaderModules() const;
    VkPipelineLayout getPipelineLayout() const;
    VkDescriptorSet getDescriptorSet(size_t index) const;

    // TODO: Refactor this method to update individual matrix, or even move it to mesh
    void updateMatrices(MatricesUBO matrices) const;

private:
    void createPipelineLayout();
    void deletePipelineLayout();

    void createDescriptorSetLayout();
    void deleteDescriptorSetLayout();

    void createTextureImage();
    void deleteTextureImage();
    void createTextureImageView();
    void deleteTextureImageView();
    void createTextureSampler();
    void deleteTextureSampler();

    void createUniformBuffers();
    void deleteUniformBuffers();

    void createDescriptorPool();
    void deleteDescriptorPool();
    void createDescriptorSets();
    void deleteDescriptorSets();

    VkShaderModule createShaderModule(const std::vector<char> &code) const;

private:
    MaterialSettings _materialSettings;

    VkDevice _device;
    const VulkanUtils& _vulkanUtils;
    mutable std::vector<VkShaderModule> _shaderModules;

    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

    uint32_t _mipLevels;
    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;

    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
};

} // namespace SVE