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
    std::vector<SamplerInfo> getPostEffectSamplerInfo(uint32_t effectId) const;
    void setSamplerInfo(TextureType type, std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfoList, int subtype = 0);

private:
    std::map<TextureType, std::vector<SamplerInfo>> _samplerMap;
    std::map<uint32_t, std::vector<SamplerInfo>> _postEffectMap;
    int _lastEffect = -1;
};

using VulkanSamplerInfoList = std::vector<VulkanSamplerHolder::SamplerInfo>;

} // namespace SVE