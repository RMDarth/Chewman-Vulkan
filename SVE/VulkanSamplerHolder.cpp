// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanSamplerHolder.h"
#include "VulkanException.h"

namespace SVE
{

std::vector<VulkanSamplerHolder::SamplerInfo> VulkanSamplerHolder::getSamplerInfo(TextureType type) const
{
    if (type == TextureType::LastEffect)
    {
        if (_lastEffect < 0)
            return getSamplerInfo(TextureType::ScreenQuad);
        return getPostEffectSamplerInfo(_lastEffect);
    }
    auto samplerIter = _samplerMap.find(type);
    if (samplerIter == _samplerMap.end())
    {
        return std::vector<VulkanSamplerHolder::SamplerInfo>();
    }
    return samplerIter->second;
}

std::vector<VulkanSamplerHolder::SamplerInfo> VulkanSamplerHolder::getPostEffectSamplerInfo(uint32_t effectId) const
{
    auto samplerIter = _postEffectMap.find(effectId);
    if (samplerIter == _postEffectMap.end())
    {
        return std::vector<VulkanSamplerHolder::SamplerInfo>();
    }
    return samplerIter->second;
}

void VulkanSamplerHolder::setSamplerInfo(
        TextureType type,
        std::vector<VulkanSamplerHolder::SamplerInfo> samplerInfo,
        int subtype)
{
    if (type == TextureType::ScreenQuad && subtype != 0)
    {
        _postEffectMap.emplace(subtype, samplerInfo);
        _lastEffect = subtype;
    }
    else
    {
        _samplerMap[type] = std::move(samplerInfo);
    }
}


} // namespace SVE