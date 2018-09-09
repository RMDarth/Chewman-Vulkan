// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include "VulkanUtils.h"
#include "ShaderSettings.h"

namespace SVE
{

class VulkanShaderInfo
{
public:
    VulkanShaderInfo(ShaderSettings shaderSettings);
    ~VulkanShaderInfo();

    VkPipelineShaderStageCreateInfo createShaderStage();
    void freeShaderModule();

    size_t getShaderUniformsSize() const;
    const ShaderSettings& getShaderSettings() const;

    VkDescriptorSetLayout getDescriptorSetLayout() const;
private:
    void createDescriptorSetLayout();
    void deleteDescriptorSetLayout();

    VkShaderModule createShaderModule(const std::vector<char> &code) const;

private:
    VkDevice _device;
    ShaderSettings _shaderSettings;
    VkShaderStageFlagBits _shaderStage;
    VkShaderModule _shaderModule;

    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
};

} // namespace SVE