// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <fstream>
#include <SVE/VulkanException.h>
#include "GraphicsSettings.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "SVE/ResourceManager.h"

namespace Chewman
{
namespace
{

std::string getSettingsPath()
{
    auto settingsPath = SVE::Engine::getInstance()->getResourceManager()->getSavePath();
    if (settingsPath.back() != '/' || settingsPath.back() != '\\')
        settingsPath.push_back('/');
    settingsPath += "settings.dat";
    return settingsPath;
}

} // namespace

std::string getResolutionText(ResolutionSettings resolutionSettings)
{
    switch(resolutionSettings)
    {
        case ResolutionSettings::Low: return "Low";
        case ResolutionSettings::High: return "High";
        case ResolutionSettings::Native: return "Native";
        case ResolutionSettings::Custom: return "Custom";
    }

    assert(!"Incorrect resolution settings");
    return "Unknown";
}

std::string getEffectText(EffectSettings effectSettings)
{
    switch (effectSettings)
    {
        case EffectSettings::Low: return "Low";
        case EffectSettings::High: return "High";
    }

    assert(!"Incorrect effects settings");
    return "Unknown";
}

GraphicsManager::GraphicsManager()
{
    load();
}

void GraphicsManager::setSettings(GraphicsSettings settings)
{
    auto* engine = SVE::Engine::getInstance();
    _currentSettings = settings;

    auto sunLight = engine->getSceneManager()->getLightManager()->getDirectionLight();
    sunLight->getLightSettings().castShadows = _currentSettings.useShadows;

    store();
}

const GraphicsSettings& GraphicsManager::getSettings() const
{
    return _currentSettings;
}

void GraphicsManager::store()
{
    std::ofstream fout(getSettingsPath());
    if (!fout)
    {
        throw SVE::VulkanException("Can't save settings file");
    }

    fout.write(reinterpret_cast<const char*>(&_currentSettings), sizeof(_currentSettings));
    fout.close();
}

void GraphicsManager::load()
{
    std::ifstream fin(getSettingsPath());
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }
    fin.read(reinterpret_cast<char*>(&_currentSettings), sizeof(_currentSettings));
    fin.close();
}

} // namespace Chewman