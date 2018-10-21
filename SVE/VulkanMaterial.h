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
class VulkanInstance;
class Entity;

class VulkanMaterial
{
public:
    explicit VulkanMaterial(MaterialSettings materialSettings);
    ~VulkanMaterial();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

    void applyDrawingCommands(uint32_t bufferIndex, uint32_t materialIndex);

    void resetDescriptorSets();
    void resetPipeline();

    std::vector<VkDescriptorSet> getDescriptorSets(uint32_t materialIndex, size_t index) const;

    uint32_t getInstanceForEntity(Entity* entity, uint32_t index = 0);
    bool isSkeletal() const;

    void setUniformData(uint32_t materialIndex, const UniformData& data) const;

private:
    void createPipelineLayout();
    void deletePipelineLayout();

    void createPipeline();
    void deletePipeline();

    void createTextureImages();
    void createCubemapTextureImages();
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

    void updateDescriptorSets();
    void updateDescriptorSet(uint32_t imageIndex,
                             const VkBuffer* shaderBuffer,
                             const size_t uniformSize,
                             const VulkanShaderInfo* shaderInfo,
                             VkDescriptorSet descriptorSet);

    std::vector<char> getUniformDataByType(const UniformData& data, UniformType type) const;


private:
    MaterialSettings _materialSettings;

    VulkanInstance* _vulkanInstance;
    VkDevice _device;
    const VulkanUtils& _vulkanUtils;

    VulkanShaderInfo* _vertexShader = nullptr;
    VulkanShaderInfo* _fragmentShader = nullptr;
    VulkanShaderInfo* _geometryShader = nullptr;
    std::vector<VulkanShaderInfo*> _shaderList;

    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
    VkPipeline _pipeline;

    std::vector<uint32_t> _mipLevels;
    std::vector<VkImage> _textureImages;
    std::vector<VkDeviceMemory> _textureImageMemoryList;
    std::vector<VkImageView> _textureImageViews;
    std::vector<VkSampler> _textureSamplers;
    std::vector<std::string> _textureNames;

    bool _hasExternals;
    std::vector<bool> _textureExternal;
    std::vector<std::string> _textureExternalName;

    struct PerInstanceData
    {
        std::vector<VkBuffer> vertexUniformBuffers;
        std::vector<VkBuffer> fragmentUniformBuffer;
        std::vector<VkBuffer> geometryUniformBuffer;
        std::vector<VkDeviceMemory> uniformBuffersMemory;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        std::vector<VkDescriptorSet> vertexDescriptorSets;
        std::vector<VkDescriptorSet> fragmentDescriptorSets;
        std::vector<VkDescriptorSet> geometryDescriptorSets;
    };

    std::map<Entity*, std::vector<uint32_t>> _entityInstanceMap;
    std::vector<PerInstanceData> _instanceData;
};

} // namespace SVE