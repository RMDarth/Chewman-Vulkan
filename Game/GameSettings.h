// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once

namespace Chewman
{

constexpr uint8_t CurrentGameSettingsVersion = 5;

enum class CameraStyle : uint8_t
{
    Horizontal,
    Balanced,
    Vertical
};

enum class ControllerType : uint8_t
{
    Swipe,
    Joystick,
    Accelerometer
};

struct GameSettings
{
    uint8_t version = CurrentGameSettingsVersion;
    ControllerType controllerType = ControllerType::Swipe;
    bool switchLight[36] = {};
    float brightness = 0.5f;
    CameraStyle cameraStyle = CameraStyle::Horizontal;
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