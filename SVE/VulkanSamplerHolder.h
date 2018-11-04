// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "MaterialSettings.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <map>


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
    std::vector<SamplerInfo> getSamplerInfo(TextureType type) const;
    void setSamplerInfo(TextureType type, std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfoList);

private:
    std::map<TextureType, std::vector<SamplerInfo>> _samplerMap;

};

using VulkanSamplerInfoList = std::vector<VulkanSamplerHolder::SamplerInfo>;

} // namespace SVE