// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanSamplerHolder.h"
#include "VulkanException.h"

namespace SVE
{

std::vector<VulkanSamplerHolder::SamplerInfo> VulkanSamplerHolder::getSamplerInfo(const std::string& name) const
{
    auto samplerIter = _samplerMap.find(name);
    if (samplerIter == _samplerMap.end())
    {
        return std::vector<VulkanSamplerHolder::SamplerInfo>();
        //throw VulkanException(std::string("Sampler ") + name + " not found");
    }
    return samplerIter->second;
}

void VulkanSamplerHolder::setSamplerInfo(
        const std::string& name,
        std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfo)
{
    _samplerMap.insert({name, samplerInfo});
}


} // namespace SVE