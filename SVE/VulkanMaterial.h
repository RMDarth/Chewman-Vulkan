// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Libs.h"
#include "MaterialSettings.h"
#include "ShaderSettings.h"

namespace SVE
{
class VulkanUtils;
class VulkanShaderInfo;

class VulkanMaterial
{
public:
    explicit VulkanMaterial(MaterialSettings materialSettings);
    ~VulkanMaterial();

    VkPipelineLayout getPipelineLayout() const;
    std::vector<VkDescriptorSet> getDescriptorSets(size_t index) const;
    std::vector<VkPipelineShaderStageCreateInfo> getShaderStages() const;
    void freeShaderModules() const;

    void setUniformData(UniformData data) const;

private:
    void createPipelineLayout();
    void deletePipelineLayout();

    void createTextureImages();
    void deleteTextureImages();
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

    std::vector<char> getUniformDataByType(const UniformData& data, UniformType type) const;

private:
    MaterialSettings _materialSettings;

    VkDevice _device;
    const VulkanUtils& _vulkanUtils;

    VulkanShaderInfo* _vertexShader = nullptr;
    VulkanShaderInfo* _fragmentShader = nullptr;
    VulkanShaderInfo* _geometryShader = nullptr;
    std::vector<VulkanShaderInfo*> _shaderList;

    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

    std::vector<uint32_t> _mipLevels;
    std::vector<VkImage> _textureImages;
    std::vector<VkDeviceMemory> _textureImageMemoryList;
    std::vector<VkImageView> _textureImageViews;
    std::vector<VkSampler> _textureSamplers;
    std::vector<std::string> _textureNames;

    std::vector<VkBuffer> _vertexUniformBuffers;
    std::vector<VkBuffer> _fragmentUniformBuffer;
    std::vector<VkBuffer> _geometryUniformBuffer;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    VkDescriptorPool _descriptorPool;

    std::vector<VkDescriptorSet> _vertexDescriptorSets;
    std::vector<VkDescriptorSet> _fragmentDescriptorSets;
    std::vector<VkDescriptorSet> _geometryDescriptorSets;

};

} // namespace SVE