// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "LightNode.h"
#include <vector>

namespace SVE
{

class LightManager
{
public:
    void setLight(std::shared_ptr<LightNode> light, uint16_t index);
    std::shared_ptr<LightNode> getLight(uint16_t index) const;
    size_t getLightCount() const;

    void fillUniformData(UniformData& data, int viewSource = -1);

private:
    std::vector<std::shared_ptr<LightNode>> _lightList;
};

} // namespace SVE