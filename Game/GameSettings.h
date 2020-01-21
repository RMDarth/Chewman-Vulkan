// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

namespace Chewman
{

constexpr uint8_t CurrentGameSettingsVersion = 3;

struct GameSettings
{
    uint8_t version = CurrentGameSettingsVersion;
    bool showOnScreenControls = false;
    bool switchLight[36] = {};
    float brightness = 0.5f;
};

class GameSettingsManager
{
public:
    GameSettingsManager();
    GameSettingsManager(const GameSettingsManager&) = delete;

    GameSettings& getSettings();

    void store();
    void load();

private:
    GameSettings _currentSettings;
};

} // namespace Chewman