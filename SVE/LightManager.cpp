// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
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

LightManager::LightManager(bool useCascadeShadowMap)
    : _useCascadeShadowMap(useCascadeShadowMap)
    , _directLightShadowMap(std::make_shared<ShadowMap>(LightType::SunLight, useCascadeShadowMap ? MAX_CASCADES : 1, DirectShadowSize))
    //, _pointLightShadowMap(std::make_shared<ShadowMap>(LightType::ShadowPointLight, MAX_LIGHTS * 6, PointShadowSize))
{
}

LightManager::~LightManager() = default;

void LightManager::addLight(LightNode* light)
{
    if (light->getLightSettings().lightType == LightType::SunLight)
    {
        _directLight = light;
    }
    else
    {
        if (light->getLightSettings().lightType == LightType::ShadowPointLight)
        {
            _usePointLightShadow = true;
        }

        _lightList.insert(light);
    }
}

void LightManager::removeLight(LightNode* light)
{
    _lightList.erase(light);
}

LightNode* LightManager::getLight(uint32_t index) const
{
    assert(index < _lightList.size());
    auto iter = _lightList.begin();
    while(index)
    {
        --index;
        ++iter;
    }
    return *iter;
}

LightNode* LightManager::getDirectionLight() const
{
    return _directLight;
}

size_t LightManager::getLightCount() const
{
    return _lightList.size();
}

std::shared_ptr<ShadowMap> LightManager::getPointLightShadowMap()
{
    if (!_usePointLightShadow)
        return std::shared_ptr<ShadowMap>();
    return _pointLightShadowMap;
}

std::shared_ptr<ShadowMap> LightManager::getDirectLightShadowMap()
{
    if (!_directLight)
        return std::shared_ptr<ShadowMap>();
    return _directLightShadowMap;
}

void LightManager::fillUniformData(UniformData& data, LightType viewSourceLightType)
{
    data.lightPointViewProjectionList.resize(MAX_LIGHTS);
    auto activeLights = 0;
    for (auto& light : _lightList)
    {
        if (light && light->getCurrentFrame() == _currentFrame)
        {
            if (viewSourceLightType == LightType::ShadowPointLight && !light->castShadows())
                continue;
            light->fillUniformData(data, activeLights + 1, false);
            ++activeLights;
        }
    }
    if (_directLight)
        _directLight->fillUniformData(data, 0, false);

    if (viewSourceLightType == LightType::SunLight)
    {
        assert(_directLight);
        _directLight->fillUniformData(data, 0, true);
    } else if (viewSourceLightType == LightType::ShadowPointLight)
    {
        activeLights = 0;
        for (auto& light : _lightList)
        {
            if (light && light->getCurrentFrame() == _currentFrame)
            {
                if (light->castShadows())
                    light->fillUniformData(data, activeLights + 1, true);
                ++activeLights;
            }
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
    data.pointLightList.resize(20);
}

void LightManager::setCurrentFrame(uint64_t frame)
{
    _currentFrame = frame;
}

} // namespace SVE