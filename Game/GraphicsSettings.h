// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>

namespace Chewman
{

enum class ResolutionSettings
{
    Low,
    High,
    Native,
    Custom
};

enum class EffectSettings
{
    Low,
    High
};

struct GraphicsSettings
{
    ResolutionSettings resolution = ResolutionSettings::High;
    bool useShadows = true;
    bool useDynamicLights = true;
    EffectSettings effectSettings = EffectSettings::High;
};

std::string getResolutionText(ResolutionSettings resolutionSettings);
std::string getEffectText(EffectSettings effectSettings);

class GraphicsManager
{
public:
    GraphicsManager();

    void setSettings(GraphicsSettings settings);
    const GraphicsSettings& getSettings() const;

    void store();
    void load();
private:
    GraphicsSettings _currentSettings;
};

} // namespace Chewman