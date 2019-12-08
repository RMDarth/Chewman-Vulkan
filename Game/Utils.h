// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>

namespace Chewman
{
namespace Utils
{

std::string getSettingsPath(const std::string& filename);
std::string timeToString(uint32_t time);

} // namespace Utils
} // namespace Chewman