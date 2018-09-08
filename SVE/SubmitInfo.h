// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vulkan/vulkan.h>

namespace SVE
{

class SubmitInfo
{
public:
    explicit SubmitInfo(VkSubmitInfo submitInfo);

    const VkSubmitInfo& getVulkanSubmitInfo() const;
private:
    VkSubmitInfo _submitInfo;
};

} // namespace SVE