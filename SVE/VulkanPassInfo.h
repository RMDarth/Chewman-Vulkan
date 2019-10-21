// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanHeaders.h"
#include "Engine.h"
#include <map>

namespace SVE
{

class VulkanPassInfo
{
public:
    struct PassData
    {
        VkRenderPass renderPass;
    };

    const PassData& getPassData(CommandsType pass) const;
    void setPassData(CommandsType pass, PassData passData);

private:
    std::map<CommandsType, PassData> _passDataMap;
};

} // namespace SVE