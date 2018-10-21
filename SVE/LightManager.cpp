// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightManager.h"
#include "ShaderSettings.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanSamplerHolder.h"
#include "VulkanShadowImage.h"
#include <utility>

namespace SVE
{
static const std::string ShadowSamplerName = "shadowmap";
static const uint32_t MAX_LIGHTS = 6;
static const uint32_t ShadowSize = 4096;

LightManager::LightManager()
    : _shadowImage(std::make_unique<VulkanShadowImage>(MAX_LIGHTS, ShadowSize))
{
    VulkanSamplerInfoList list;
    for (auto i = 0; i < Engine::getInstance()->getVulkanInstance()->getSwapchainSize(); i++)
    {
        VulkanSamplerHolder::SamplerInfo samplerInfo{
                _shadowImage->getImageView(i),
                _shadowImage->getSampler(i)
        };
        list.push_back(samplerInfo);
    }
    Engine::getInstance()->getVulkanInstance()->getSamplerHolder()->setSamplerInfo(ShadowSamplerName, list);
}

LightManager::~LightManager() = default;

void LightManager::setLight(std::shared_ptr<LightNode> light, uint16_t index)
{
    if (_lightList.size() <= index)
        _lightList.resize(index + 1);
    _lightList[index] = std::move(light);
}

std::shared_ptr<LightNode> LightManager::getLight(uint16_t index) const
{
    assert(index < _lightList.size());
    return _lightList[index];
}

size_t LightManager::getLightCount() const
{
    return _lightList.size();
}

void LightManager::fillUniformData(UniformData& data, int viewSource)
{
    // TODO: Make 6 some const
    data.lightViewProjectionList.resize(MAX_LIGHTS);
    for (auto i = 0u; i < _lightList.size(); i++)
    {
        if (_lightList[i])
            _lightList[i]->fillUniformData(data, i, viewSource == i);
    }
    // TODO: Replace 4 with constant or data from shader
    if (data.pointLightList.size() != 4)
    {
        // TODO: Should sort and give closest 4
        // TODO: Refactor to remove dummy lights if light count < 4
        data.pointLightList.resize(4);
    }
}

VulkanShadowImage* LightManager::getVulkanShadowImage()
{
    return _shadowImage.get();
}

} // namespace SVE