// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightManager.h"
#include "ShaderSettings.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanSamplerHolder.h"
#include "ShadowMap.h"
#include <utility>

namespace SVE
{
static const uint32_t MAX_LIGHTS = 3;
static const uint32_t DirectShadowSize = 4096;
static const uint32_t PointShadowSize = 512;

LightManager::LightManager()
    : _directLightShadowMap(std::make_shared<ShadowMap>(LightType::SunLight, MAX_CASCADES, DirectShadowSize))
    , _pointLightShadowMap(std::make_shared<ShadowMap>(LightType::ShadowPointLight, MAX_LIGHTS * 6, PointShadowSize))
{
}

LightManager::~LightManager() = default;

void LightManager::setLight(std::shared_ptr<LightNode> light, uint32_t index)
{
    if (light->getLightSettings().lightType == LightType::SunLight)
    {
        _directLight = std::move(light);
    }
    else
    {
        if (_lightList.size() <= index)
            _lightList.resize(index + 1);

        _lightList[index] = std::move(light);
    }
}

std::shared_ptr<LightNode> LightManager::getLight(uint32_t index) const
{
    assert(index < _lightList.size());
    return _lightList[index];
}

std::shared_ptr<LightNode> LightManager::getDirectionLight() const
{
    return _directLight;
}

size_t LightManager::getLightCount() const
{
    return _lightList.size();
}

std::shared_ptr<ShadowMap> LightManager::getPointLightShadowMap()
{
    return _pointLightShadowMap;
}

std::shared_ptr<ShadowMap> LightManager::getDirectLightShadowMap()
{
    return _directLightShadowMap;
}

void LightManager::fillUniformData(UniformData& data, LightType viewSourceLightType)
{
    data.lightPointViewProjectionList.resize(MAX_LIGHTS);
    for (auto i = 0u; i < _lightList.size(); i++)
    {
        if (_lightList[i])
        {
            if (viewSourceLightType == LightType::ShadowPointLight && !_lightList[i]->castShadows())
                continue;
            _lightList[i]->fillUniformData(data, i + 1, false);
        }
    }
    _directLight->fillUniformData(data, 0, false);


    if (viewSourceLightType == LightType::SunLight)
    {
        _directLight->fillUniformData(data, 0, true);
    } else if (viewSourceLightType == LightType::ShadowPointLight)
    {
        for (auto i = 0u; i < _lightList.size(); i++)
        {
            if (_lightList[i] && _lightList[i]->castShadows())
                _lightList[i]->fillUniformData(data, i + 1, true);
        }
    }

    // TODO: Replace 4 with constant or data from shader
    if (data.shadowPointLightList.size() != 4)
    {
        // TODO: Should sort and give closest 4
        // TODO: Refactor to remove dummy lights if light count < 4
        data.shadowPointLightList.resize(4);
    }

    data.lineLightList.resize(15);
    data.pointLightList.resize(10);
}

void LightManager::removeLight(uint32_t index)
{
    _lightList.erase(_lightList.begin() + index);
}

} // namespace SVE