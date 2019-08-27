// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

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

class RapidJsonException : public VulkanException
{
public:
    RapidJsonException()
            : VulkanException("Error parsing resource")
    {}
};

#define RAPIDJSON_ASSERT(x) if(!(x)) throw SVE::RapidJsonException();

} // namespace SVE