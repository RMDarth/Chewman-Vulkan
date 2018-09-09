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
    SubmitInfo();

    bool isEmpty();
    const VkSubmitInfo& getVulkanSubmitInfo() const;
private:
    bool _empty;
    VkSubmitInfo _submitInfo;
};

} // namespace SVE