// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>

namespace SVE
{

class VulkanWater;

class Water
{
public:
    explicit Water(float height);
    ~Water();

    VulkanWater* getVulkanWater();

private:
    std::unique_ptr<VulkanWater> _vulkanWater;
};

} // namespace SVE