// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include "VulkanException.h"

namespace SVE
{

VulkanException::VulkanException(const char* error)
    : runtime_error(error)
{

}

VulkanException::VulkanException(const std::string& error)
    : runtime_error(error)
{

}

} // namespace SVE