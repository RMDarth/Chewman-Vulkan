// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ShadowMap.h"
#include "VulkanPointShadowMap.h"
#include "VulkanDirectShadowMap.h"
#include "Engine.h"
#include "SceneManager.h"
#include "LightManager.h"
#include "LightSettings.h"
#include "VulkanException.h"

namespace SVE
{

namespace
{

std::unique_ptr<VulkanCommandsManager> createVulkanShadowMap(
        LightType lightType, uint32_t layersCount, uint32_t shadowMapSize)
{
    switch (lightType)
    {
        case LightType::PointLight:
            return std::make_unique<VulkanPointShadowMap>(layersCount, shadowMapSize);
        case LightType::SunLight:
            return std::make_unique<VulkanDirectShadowMap>(layersCount, shadowMapSize);
        case LightType::SpotLight:
            throw VulkanException("Spot light currently unsupported");
        case LightType::RectLight:
            throw VulkanException("Rect light currently unsupported");
    }
    throw VulkanException("Unknown light type");
}

}

ShadowMap::ShadowMap(LightType lightType, uint32_t layersCount, uint32_t shadowMapSize)
    : _vulkanShadowMap(createVulkanShadowMap(lightType, layersCount, shadowMapSize))
{
}

ShadowMap::~ShadowMap() = default;

VulkanCommandsManager* ShadowMap::getVulkanShadowMap()
{
    return _vulkanShadowMap.get();
}

} // namespace SVE