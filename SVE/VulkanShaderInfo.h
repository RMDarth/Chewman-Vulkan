// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanHeaders.h"
#include "VulkanUtils.h"
#include "ShaderSettings.h"
#include <vector>
#include <map>

namespace SVE
{

class VulkanShaderInfo
{
public:
    VulkanShaderInfo(ShaderSettings shaderSettings);
    ~VulkanShaderInfo();

    VkPipelineShaderStageCreateInfo createShaderStage();
    void freeShaderModule();

    std::vector<VkVertexInputBindingDescription> getBindingDescription() const;
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() const;

    size_t getShaderUniformsSize() const;
    size_t getShaderStorageBuffersSize() const;
    const ShaderSettings& getShaderSettings() const;

    VkDescriptorSetLayout getDescriptorSetLayout() const;
private:
    void createDescriptorSetLayout();
    void deleteDescriptorSetLayout();

    VkShaderModule createShaderModule(const std::string& code) const;
    uint32_t getVertexDataSize(VertexInfo::VertexDataType vertexDataType) const;

private:
    VkDevice _device;
    ShaderSettings _shaderSettings;
    VkShaderStageFlagBits _shaderStage;
    VkShaderModule _shaderModule;

    VkDescriptorSetLayout _descriptorSetLayout;
};

} // namespace SVE