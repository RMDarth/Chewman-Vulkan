// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ShadowMap.h"
#include "VulkanShadowMap.h"

namespace SVE
{

ShadowMap::ShadowMap()
    : _isEnabled(false)
    , _vulkanShadowMap(std::make_unique<VulkanShadowMap>())
{
}

ShadowMap::~ShadowMap() = default;

VulkanShadowMap* ShadowMap::getVulkanShadowMap()
{
    return _vulkanShadowMap.get();
}

void ShadowMap::setMaterial(std::string materialName)
{
    _isEnabled = true;
    _vulkanShadowMap->setMaterial(materialName);
}

bool ShadowMap::isEnabled()
{
    return _isEnabled;
}

} // namespace SVE