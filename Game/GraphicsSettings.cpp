// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <fstream>
#include <SVE/VulkanException.h>
#include <SVE/VulkanInstance.h>
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
        case ResolutionSettings::Low: return "720p";
        case ResolutionSettings::High: return "1080p";
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

std::string getParticlesText(ParticlesSettings settings)
{
    switch (settings)
    {
        case ParticlesSettings::Full: return "Full";
        case ParticlesSettings::Partial: return "Partial";
        case ParticlesSettings::None: return "None";
    }

    assert(!"Incorrect particle effect settings");
    return "Unknown";
}

GraphicsManager::GraphicsManager()
{
    load();

    if (SVE::Engine::getInstance()->isFirstRun())
    {
        tuneSettings();
    }
}

void GraphicsManager::setSettings(GraphicsSettings settings)
{
    auto* engine = SVE::Engine::getInstance();
    _currentSettings = settings;

    auto sunLight = engine->getSceneManager()->getLightManager()->getDirectionLight();
    sunLight->getLightSettings().castShadows = _currentSettings.useShadows;
    engine->getVulkanInstance()->disableParticles(_currentSettings.particleEffects == ParticlesSettings::None);

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
    if (_currentSettings.version != CurrentGraphicsSettingsVersion)
    {
        _currentSettings = {};
    }
    SVE::Engine::getInstance()->getVulkanInstance()->disableParticles(_currentSettings.particleEffects == ParticlesSettings::None);

    fin.close();
}

void GraphicsManager::tuneSettings()
{
    auto gpuInfo = SVE::Engine::getInstance()->getVulkanInstance()->getGPUInfo();

    std::string deviceName = gpuInfo.deviceName;
    if (deviceName.find("Adreno") != std::string::npos)
    {
        int model = 0;
        if (sscanf(gpuInfo.deviceName, "Adreno (TM) %d", &model) != EOF)
        {
            if (_currentSettings.effectSettings == EffectSettings::Unknown)
                _currentSettings.effectSettings = EffectSettings::High;

            if (model < 630)
            {
                _currentSettings.effectSettings = EffectSettings::Low;
                _currentSettings.useDynamicLights = false;
            }
            // if (model <= 540) & Android version < 8.0 throw exception or show warning
            if (model < 540)
            {
                _currentSettings.resolution = ResolutionSettings::Low;
            }
        }
    }
    if (deviceName.find("Mali") != std::string::npos)
    {
        if (_currentSettings.effectSettings != EffectSettings::Unknown)
            _currentSettings.effectSettings = EffectSettings::High;

        if (deviceName.find("Mali-G") != std::string::npos)
        {
            int model = 0;
            if (sscanf(gpuInfo.deviceName, "Mali-G%d", &model) != EOF)
            {
                if (_currentSettings.effectSettings == EffectSettings::Unknown)
                    _currentSettings.effectSettings = EffectSettings::High;

                if (model == 72)
                {
                    if (deviceName.find("Samsung") == std::string::npos)
                    {
                        _currentSettings.effectSettings = EffectSettings::Low;
                    }
                }
                if (model < 72)
                {
                    // TODO: If AndroidVersion < 8.0 (API 26) then throw exception or show warning
                    _currentSettings.effectSettings = EffectSettings::Low;
                    _currentSettings.useDynamicLights = false;
                }
                if (model < 57)
                {
                    _currentSettings.resolution = ResolutionSettings::Low;
                }

            }
        } else if (deviceName.find("Mali-T") != std::string::npos)
        {
            _currentSettings.effectSettings = EffectSettings::Low;
            _currentSettings.useDynamicLights = false;
            _currentSettings.resolution = ResolutionSettings::Low;
        }
    }

    if (_currentSettings.effectSettings == EffectSettings::Unknown)
    {
        _currentSettings.effectSettings = EffectSettings::Low;
        _currentSettings.useDynamicLights = false;

        if (gpuInfo.limits.maxPerStageDescriptorSamplers < 100)
        {
            _currentSettings.resolution = ResolutionSettings::Low;
        }
    }

    store();
}

} // namespace Chewman