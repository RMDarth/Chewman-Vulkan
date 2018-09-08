// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include <utility>
#include "SubmitInfo.h"

namespace SVE
{

SubmitInfo::SubmitInfo(VkSubmitInfo submitInfo)
    : _submitInfo(std::move(submitInfo))
{

}

const VkSubmitInfo& SubmitInfo::getVulkanSubmitInfo() const
{
    return _submitInfo;
}
} // namespace SVE