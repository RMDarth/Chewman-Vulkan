// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include "VulkanException.h"
#include <string>

namespace SVE
{

namespace
{

std::string getVkResultStr(VkResult result)
{
    #define ERROR_CASE_STR(error) case VK_##error: return #error;
    switch (result)
    {
        ERROR_CASE_STR(NOT_READY)
        ERROR_CASE_STR(TIMEOUT)
        ERROR_CASE_STR(EVENT_SET)
        ERROR_CASE_STR(EVENT_RESET)
        ERROR_CASE_STR(INCOMPLETE)
        ERROR_CASE_STR(ERROR_OUT_OF_HOST_MEMORY)
        ERROR_CASE_STR(ERROR_OUT_OF_DEVICE_MEMORY)
        ERROR_CASE_STR(ERROR_INITIALIZATION_FAILED)
        ERROR_CASE_STR(ERROR_DEVICE_LOST)
        ERROR_CASE_STR(ERROR_MEMORY_MAP_FAILED)
        ERROR_CASE_STR(ERROR_LAYER_NOT_PRESENT)
        ERROR_CASE_STR(ERROR_EXTENSION_NOT_PRESENT)
        ERROR_CASE_STR(ERROR_FEATURE_NOT_PRESENT)
        ERROR_CASE_STR(ERROR_INCOMPATIBLE_DRIVER)
        ERROR_CASE_STR(ERROR_TOO_MANY_OBJECTS)
        ERROR_CASE_STR(ERROR_FORMAT_NOT_SUPPORTED)
        ERROR_CASE_STR(ERROR_SURFACE_LOST_KHR)
        ERROR_CASE_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR)
        ERROR_CASE_STR(SUBOPTIMAL_KHR)
        ERROR_CASE_STR(ERROR_OUT_OF_DATE_KHR)
        ERROR_CASE_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR)
        ERROR_CASE_STR(ERROR_VALIDATION_FAILED_EXT)
        ERROR_CASE_STR(ERROR_INVALID_SHADER_NV)
        default:
            return "UNKNOWN_ERROR";
    }
    #undef ERROR_CASE_STR
}

std::string addPossibleResult(VkResult result)
{
    if (result != VK_RESULT_MAX_ENUM)
    {
        return ", VkResult = " + getVkResultStr(result);
    }
    return "";
}

}

VulkanException::VulkanException(const char* error, VkResult result)
    : runtime_error(std::string(error) + addPossibleResult(result))
{

}

VulkanException::VulkanException(const std::string& error, VkResult result)
    : runtime_error(error + addPossibleResult(result))
{

}

} // namespace SVE