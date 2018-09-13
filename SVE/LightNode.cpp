// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightNode.h"
#include "ShaderSettings.h"

namespace SVE
{

LightNode::LightNode(LightSettings lightSettings)
    : _lightSettings(std::move(lightSettings))
{

}

const LightSettings& LightNode::getLightSettings()
{
    return _lightSettings;
}

void LightNode::fillUniformData(UniformData& data)
{
    data.lightPos = getTotalTransformation()[3];
    data.lightSettings = _lightSettings;
}


} // namespace SVE