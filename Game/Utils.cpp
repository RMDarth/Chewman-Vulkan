// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Utils.h"
#include "SVE/Engine.h"
#include "SVE/ResourceManager.h"

#include <sstream>
#include <iomanip>

namespace Chewman
{
namespace Utils
{

std::string getSettingsPath(const std::string& filename)
{
    auto settingsPath = SVE::Engine::getInstance()->getResourceManager()->getSavePath();
    if (settingsPath.back() != '/' || settingsPath.back() != '\\')
        settingsPath.push_back('/');
    settingsPath += filename;
    return settingsPath;
}

std::string timeToString(uint32_t time)
{
    std::stringstream stream;
    stream << time / 60 << ":" << std::setfill('0') << std::setw(2) << time % 60;
    return stream.str();
}

} // namespace Utils
} // namespace Chewman