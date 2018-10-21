// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace SVE
{

class VulkanSamplerHolder
{
public:
    struct SamplerInfo
    {
        VkImageView imageView;
        VkSampler sampler;
    };

    // info for each swapchain image
    std::vector<SamplerInfo> getSamplerInfo(const std::string& name) const;
    void setSamplerInfo(const std::string& name, std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfoList);

private:
    std::unordered_map<std::string, std::vector<SamplerInfo>> _samplerMap;

};

using VulkanSamplerInfoList = std::vector<VulkanSamplerHolder::SamplerInfo>;

} // namespace SVE