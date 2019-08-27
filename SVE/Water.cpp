// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Water.h"
#include "VulkanWater.h"

namespace SVE
{

Water::Water(float height)
    : _vulkanWater(std::make_unique<VulkanWater>(height))
{

}

Water::~Water() = default;

VulkanWater* Water::getVulkanWater()
{
    return _vulkanWater.get();
}
} // namespace SVE