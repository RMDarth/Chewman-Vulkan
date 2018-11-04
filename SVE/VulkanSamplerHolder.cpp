// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanSamplerHolder.h"
#include "VulkanException.h"

namespace SVE
{

std::vector<VulkanSamplerHolder::SamplerInfo> VulkanSamplerHolder::getSamplerInfo(TextureType type) const
{
    auto samplerIter = _samplerMap.find(type);
    if (samplerIter == _samplerMap.end())
    {
        return std::vector<VulkanSamplerHolder::SamplerInfo>();
    }
    return samplerIter->second;
}

void VulkanSamplerHolder::setSamplerInfo(
        TextureType type,
        std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfo)
{
    _samplerMap.insert({type, samplerInfo});
}


} // namespace SVE