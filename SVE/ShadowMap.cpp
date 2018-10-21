// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ShadowMap.h"
#include "VulkanShadowMap.h"
#include "Engine.h"
#include "SceneManager.h"
#include "LightManager.h"

namespace SVE
{

ShadowMap::ShadowMap(uint32_t lightIndex)
    : _isEnabled(false)
    , _vulkanShadowMap(std::make_unique<VulkanShadowMap>(lightIndex, *Engine::getInstance()->getSceneManager()->getLightManager()->getVulkanShadowImage()))
{
}

ShadowMap::~ShadowMap() = default;

VulkanShadowMap* ShadowMap::getVulkanShadowMap()
{
    return _vulkanShadowMap.get();
}

void ShadowMap::enableShadowMap()
{
    _isEnabled = true;
}

bool ShadowMap::isEnabled()
{
    return _isEnabled;
}

} // namespace SVE