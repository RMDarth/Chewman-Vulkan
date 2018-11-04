// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ShadowMap.h"
#include "VulkanShadowMap.h"
#include "Engine.h"
#include "SceneManager.h"
#include "LightManager.h"
#include "LightSettings.h"
#include "VulkanException.h"

namespace SVE
{

namespace
{

const VulkanShadowImage& getVulkanShadowImage(LightType lightType)
{
    switch (lightType)
    {
        case LightType::PointLight:
            return *Engine::getInstance()->getSceneManager()->getLightManager()->getPointLightVulkanShadowImage();
        case LightType::SunLight:
            return *Engine::getInstance()->getSceneManager()->getLightManager()->getDirectLightVulkanShadowImage();
        case LightType::SpotLight:
            throw VulkanException("Spot light currently unsupported");
        case LightType::RectLight:
            throw VulkanException("Rect light currently unsupported");
    }
    throw VulkanException("Unknown light type");
}

}

ShadowMap::ShadowMap(LightType lightType)
    : _vulkanShadowMap(std::make_unique<VulkanShadowMap>(getVulkanShadowImage(lightType)))
{
}

ShadowMap::~ShadowMap() = default;

VulkanShadowMap* ShadowMap::getVulkanShadowMap()
{
    return _vulkanShadowMap.get();
}

} // namespace SVE