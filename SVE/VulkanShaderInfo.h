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

    VkDescriptorSetLayout getDescriptorSetLayout();
private:
    void createDescriptorSetLayout();
    void deleteDescriptorSetLayout();

    VkShaderModule createShaderModule(const std::vector<char> &code) const;

private:
    VkDevice _device;
    ShaderSettings _shaderSettings;
    VkShaderStageFlagBits _shaderStage;
    mutable VkShaderModule _shaderModule;
    std::map<std::string, int> _samplerBindingMap;

    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
};

} // namespace SVE