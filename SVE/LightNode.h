// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "LightSettings.h"
#include "SceneNode.h"

namespace SVE
{

class LightNode : public SceneNode
{
public:
    explicit LightNode(LightSettings lightSettings);

    const LightSettings& getLightSettings();

    void fillUniformData(UniformData& data);

private:
    LightSettings _lightSettings;

};

} // namespace SVE