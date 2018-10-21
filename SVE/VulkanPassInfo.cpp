// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanPassInfo.h"

namespace SVE
{

const VulkanPassInfo::PassData& VulkanPassInfo::getPassData(CommandsType pass) const
{
    return _passDataMap.at(pass);
}

void VulkanPassInfo::setPassData(CommandsType pass, VulkanPassInfo::PassData passData)
{
    _passDataMap[pass] = passData;
}
} // namespace SVE