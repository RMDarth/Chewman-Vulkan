// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>

namespace SVE
{

class VulkanWater;

class Water
{
public:
    Water(float height);
    ~Water();

    VulkanWater* getVulkanWater();

private:
    std::unique_ptr<VulkanWater> _vulkanWater;
};

} // namespace SVE