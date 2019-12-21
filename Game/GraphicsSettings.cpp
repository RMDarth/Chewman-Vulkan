// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <fstream>
#include <SVE/VulkanException.h>
#include <SVE/VulkanInstance.h>
#include "GraphicsSettings.h"
#include "SystemApi.h"
#include "Utils.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "SVE/PipelineCacheManager.h"
#include "SVE/ResourceManager.h"

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#endif

namespace Chewman
{

const std::string graphicsSettingsFile = "settings.dat";

#if defined(__ANDROID__)
std::string getSystemProperty(const char* propName)
{
    char prop[PROP_VALUE_MAX+1];
    int len = __system_property_get(propName, prop);
    if (len > 0) {
        return std::string(prop);
    } else {
        return "";
    }
}
#else
std::string getSystemProperty(const char* propName)
{
    // TODO: Get Desktop property
    return "";
}
#endif

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
        case EffectSettings::Medium: return "Medium";
        case EffectSettings::High: return "High";
    }

    assert(!"Incorrect effects settings");
    return "Unknown";
}

std::string getLightText(LightSettings lightSettings)
{
    switch (lightSettings)
    {
        case LightSettings::High: return "High";
        case LightSettings::Simple: return "Simple";
        case LightSettings::Off: return "Off";
    }

    assert(!"Incorrect light settings");
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

std::unique_ptr<GraphicsManager> GraphicsManager::_instance = {};

GraphicsManager& GraphicsManager::getInstance()
{
    if (!_instance)
    {
        _instance = std::unique_ptr<GraphicsManager>(new GraphicsManager());
    }
    return *_instance;
}

GraphicsManager::GraphicsManager()
{
    load();

    if (SVE::Engine::getInstance()->isFirstRun() || _needTune)
    {
        tuneSettings();
    }
}

void GraphicsManager::setSettings(GraphicsSettings settings)
{
    auto* engine = SVE::Engine::getInstance();

    _needRestart = changesRequireRestart(settings);
    _oldSettings = _currentSettings;
    _currentSettings = settings;

    auto sunLight = engine->getSceneManager()->getLightManager()->getDirectionLight();
    sunLight->getLightSettings().castShadows = _currentSettings.useShadows;
    engine->getVulkanInstance()->disableParticles(_currentSettings.particleEffects == ParticlesSettings::None);

    store();
}

bool GraphicsManager::changesRequireRestart(GraphicsSettings& settings)
{
    return _currentSettings.effectSettings != settings.effectSettings
           || _currentSettings.resolution != settings.resolution;
}

const GraphicsSettings& GraphicsManager::getSettings() const
{
    if (_needRestart)
        return _oldSettings;
    return _currentSettings;
}

bool GraphicsManager::needRestart() const
{
    return _needRestart;
}

void GraphicsManager::setNeedRestart(bool value)
{
    _needRestart = value;
}

void GraphicsManager::store()
{
    std::ofstream fout(Utils::getSettingsPath(graphicsSettingsFile));
    if (!fout)
    {
        throw SVE::VulkanException("Can't save settings file");
    }

    fout.write(reinterpret_cast<const char*>(&_currentSettings), sizeof(_currentSettings));
    fout.close();
}

void GraphicsManager::load()
{
    _needTune = true;
    std::ifstream fin(Utils::getSettingsPath(graphicsSettingsFile));
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }

    fin.read(reinterpret_cast<char*>(&_currentSettings), sizeof(_currentSettings));
    if (_currentSettings.version != CurrentGraphicsSettingsVersion)
    {
        _currentSettings = {};
        _needTune = true;
        SVE::Engine::getInstance()->getPipelineCacheManager()->reset();
        SVE::Engine::getInstance()->getPipelineCacheManager()->store();
    } else
    {
        _needTune = false;
    }
    SVE::Engine::getInstance()->getVulkanInstance()->disableParticles(_currentSettings.particleEffects == ParticlesSettings::None);
    _oldSettings = _currentSettings;

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
                _currentSettings.effectSettings = EffectSettings::Medium;
                _currentSettings.dynamicLights = LightSettings::Simple;
                _currentSettings.particleEffects = ParticlesSettings::None;
            }
            if (model <= 540)
            {
                if (System::getSystemVersion() < 26)
                {
                    if (!System::acceptQuary("Chewman doesn't support Android 7 on your device. Please upgrade to Android 8 or newer. You could still run the game, but it may not work or work incorrectly.", "Warning", "  Run  ", "  Exit  "))
                    {
                        throw SVE::VulkanException("Android 7 not supported");
                    }
                }
            }
            if (model < 540)
            {
                _currentSettings.effectSettings = EffectSettings::Low;
                _currentSettings.resolution = ResolutionSettings::Low;
            }
            if (model < 510)
            {
                _currentSettings.dynamicLights = LightSettings::Off;
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
                    auto manufacturer = getSystemProperty("ro.product.manufacturer");
                    if (manufacturer != "samsung")
                    {
                        _currentSettings.effectSettings = EffectSettings::Medium;
                        _currentSettings.dynamicLights = LightSettings::Simple;
                        _currentSettings.resolution = ResolutionSettings::Low;
                        _currentSettings.particleEffects = ParticlesSettings::None;
                    }
                }
                if (model < 72)
                {
                    if (System::getSystemVersion() < 26)
                    {
                        if (!System::acceptQuary("Chewman doesn't support Android 7 on your device. Please upgrade to Android 8 or newer. You could still run the game, but it may not work or work incorrectly.", "Warning", "  Run  ", "  Exit  "))
                        {
                            throw SVE::VulkanException("Android 7 not supported");
                        }
                    }
                    _currentSettings.effectSettings = EffectSettings::Medium;
                    _currentSettings.dynamicLights = LightSettings::Simple;
                    _currentSettings.resolution = ResolutionSettings::Low;
                    _currentSettings.particleEffects = ParticlesSettings::None;
                }
                if (model < 57)
                {
                    _currentSettings.effectSettings = EffectSettings::Low;
                    _currentSettings.resolution = ResolutionSettings::Low;
                }

            }
        } else if (deviceName.find("Mali-T") != std::string::npos)
        {
            _currentSettings.effectSettings = EffectSettings::Low;
            _currentSettings.dynamicLights = LightSettings::Off;
            _currentSettings.resolution = ResolutionSettings::Low;
            _currentSettings.particleEffects = ParticlesSettings::None;
        }
    }

    if (_currentSettings.effectSettings == EffectSettings::Unknown)
    {
        _currentSettings.effectSettings = EffectSettings::Medium;
        _currentSettings.dynamicLights = LightSettings::Simple;
        _currentSettings.resolution = ResolutionSettings::Low;
        _currentSettings.particleEffects = ParticlesSettings::None;

        if (gpuInfo.limits.maxPerStageDescriptorSamplers < 100)
        {
            _currentSettings.effectSettings = EffectSettings::Low;
            _currentSettings.resolution = ResolutionSettings::Low;
            _currentSettings.dynamicLights = LightSettings::Off;
        }
    }

    _oldSettings = _currentSettings;

    store();
}

bool GraphicsManager::needTune() const
{
    return _needTune;
}

} // namespace Chewman