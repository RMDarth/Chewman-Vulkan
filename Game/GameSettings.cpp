// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <fstream>
#include <iostream>
#include "GameSettings.h"
#include "Utils.h"

namespace Chewman
{

constexpr const char* const GameSettingsFile = "gameSettings.dat";

GameSettingsManager::GameSettingsManager()
{
    load();
}

GameSettings& GameSettingsManager::getSettings()
{
    return _currentSettings;
}

void GameSettingsManager::store()
{
    std::ofstream fout(Utils::getSettingsPath(GameSettingsFile));
    if (!fout)
    {
        std::cout << "Can't save game settings file" << std::endl;
        return;
    }

    fout.write(reinterpret_cast<const char*>(&_currentSettings), sizeof(_currentSettings));
    fout.close();
}

void GameSettingsManager::load()
{
    std::ifstream fin(Utils::getSettingsPath(GameSettingsFile));
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }

    fin.read(reinterpret_cast<char*>(&_currentSettings), sizeof(_currentSettings));
    if (_currentSettings.version != CurrentGameSettingsVersion)
    {
        _currentSettings = {};
    }

    fin.close();
}

} // namespace Chewman