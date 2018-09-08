// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#pragma once
#include <stdexcept>

namespace SVE
{

class VulkanException : public std::runtime_error
{
public:
    explicit VulkanException(const char* error);
    explicit VulkanException(const std::string& error);
};

} // namespace SVE