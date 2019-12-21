// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include <memory>

namespace Chewman
{

enum class ResolutionSettings : uint8_t
{
    Low,
    High,
    Native,
    Custom
};

enum class LightSettings : uint8_t
{
    Off,
    Simple,
    High
};

enum class EffectSettings : uint8_t
{
    Low,
    Medium,
    High,
    Unknown
};

enum class ParticlesSettings : uint8_t
{
    Full,
    Partial,
    None
};

constexpr uint8_t CurrentGraphicsSettingsVersion = 8;

struct GraphicsSettings
{
    uint8_t version = CurrentGraphicsSettingsVersion;
    ResolutionSettings resolution = ResolutionSettings::High;
    bool useShadows = true;
    LightSettings dynamicLights = LightSettings::High;
    ParticlesSettings particleEffects = ParticlesSettings::Partial;
    EffectSettings effectSettings = EffectSettings::Low;

    bool operator==(const GraphicsSettings& other)
    {
        return resolution == other.resolution && useShadows == other.useShadows &&
               dynamicLights == other.dynamicLights && particleEffects == other.particleEffects &&
               effectSettings == other.effectSettings;
    }
};

std::string getResolutionText(ResolutionSettings resolutionSettings);
std::string getLightText(LightSettings lightSettings);
std::string getEffectText(EffectSettings effectSettings);
std::string getParticlesText(ParticlesSettings settings);

class GraphicsManager
{
public:
    static GraphicsManager& getInstance();
    GraphicsManager(const GraphicsManager&) = delete;

    void setSettings(GraphicsSettings settings);
    const GraphicsSettings& getSettings() const;

    bool needRestart() const;
    bool needTune() const;
    void setNeedRestart(bool value = true);
    bool changesRequireRestart(GraphicsSettings& settings);

    void store();
    void load();
private:
    void tuneSettings();

private:
    GraphicsManager();
    static std::unique_ptr<GraphicsManager> _instance;

    GraphicsSettings _currentSettings;
    GraphicsSettings _oldSettings;
    bool _needRestart = false;
    bool _needTune = true;
};

} // namespace Chewman