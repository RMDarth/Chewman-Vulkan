// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightManager.h"
#include "ShaderSettings.h"
#include <utility>

namespace SVE
{

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
    for (auto i = 0u; i < _lightList.size(); i++)
    {
        if (_lightList[i])
            _lightList[i]->fillUniformData(data, viewSource == i);
    }
    // TODO: Replace 4 with constant or data from shader
    if (data.pointLightList.size() != 4)
    {
        // TODO: Should sort and give closest 4
        // TODO: Refactor to remove dummy lights if light count < 4
        data.pointLightList.resize(4);
    }
}

} // namespace SVE