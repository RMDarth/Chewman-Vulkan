// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
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

enum class TextureType : uint8_t;

class VulkanMaterial
{
public:
    explicit VulkanMaterial(MaterialSettings materialSettings);
    ~VulkanMaterial();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex, uint32_t materialIndex);

    void resetDescriptorSets();
    void updateDescriptorSets();
    void resetPipeline();

    std::vector<VkDescriptorSet> getDescriptorSets(uint32_t materialIndex, size_t index) const;

    uint32_t getInstanceForEntity(const Entity* entity, uint32_t index = 0);
    void deleteInstancesForEntity(const Entity* entity);
    bool isSkeletal() const;
    bool isMainInstance(uint32_t materialIndex) const;
    uint32_t getInstanceCount() const;
    glm::ivec2 getSpritesheetSize() const;

    const MaterialSettings& getSettings() const;

    void setUniformData(uint32_t materialIndex, const UniformData& data);
    void updateInstancedData();

private:
    struct PerInstanceData;

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
    void deleteUniformBuffers(PerInstanceData& instance);

    void createStorageBuffers();
    void deleteStorageBuffers();

    void createDescriptorPool();
    void deleteDescriptorPool();
    void deleteDescriptorPool(PerInstanceData& instance);
    void createDescriptorSets();
    void deleteDescriptorSets();
    void deleteDescriptorSets(PerInstanceData& instance);

    void updateDescriptorSet(uint32_t imageIndex,
                             const VkBuffer* shaderBuffer,
                             const VkBuffer* storageBuffer,
                             const size_t uniformSize,
                             const size_t storageSize,
                             const VulkanShaderInfo* shaderInfo,
                             VkDescriptorSet descriptorSet);

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
    struct TextureData
    {
        bool external;
        TextureType type;
        uint32_t subtype;
    };
    std::vector<TextureData> _texturesData;

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

    // for all instances
    StorageData _storageData;
    bool _storageUpdated = true;
    VkDeviceSize _storageBufferSize = 0;
    std::vector<VkBuffer> _vertexStorageBuffers;
    std::vector<VkDeviceMemory> _storageBuffersMemory;
    uint32_t _mainInstance = 0;
    uint32_t _currentInstanceCount = 0;

    std::map<const Entity*, std::vector<uint32_t>> _entityInstanceMap;
    std::vector<PerInstanceData> _instanceData;
};

} // namespace SVE