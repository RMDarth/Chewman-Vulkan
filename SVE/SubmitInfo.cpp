// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include <utility>
#include "SubmitInfo.h"

namespace SVE
{

SubmitInfo::SubmitInfo(VkSubmitInfo submitInfo)
    : _empty(false)
    , _submitInfo(std::move(submitInfo))
{
}

SubmitInfo::SubmitInfo()
        : _empty(true)
{
}

const VkSubmitInfo& SubmitInfo::getVulkanSubmitInfo() const
{
    return _submitInfo;
}

bool SubmitInfo::isEmpty()
{
    return _empty;
}

} // namespace SVE