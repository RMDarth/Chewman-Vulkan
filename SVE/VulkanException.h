// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#pragma once
#include <stdexcept>
#include "VulkanHeaders.h"

namespace SVE
{

class VulkanException : public std::runtime_error
{
public:
    explicit VulkanException(const char* error, VkResult result = VK_RESULT_MAX_ENUM );
    explicit VulkanException(const std::string& error, VkResult result = VK_RESULT_MAX_ENUM);
    VkResult getVkResult() const;

private:
    VkResult _result;

};

class RapidJsonException : public VulkanException
{
public:
    RapidJsonException()
            : VulkanException("Error parsing resource")
    {}
};

#ifndef RAPIDJSON_ASSERT
    #define RAPIDJSON_ASSERT(x) if(!(x)) throw SVE::RapidJsonException();
#endif

} // namespace SVE