// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameSoundsManager.h"
#include "SoundSystem.h"
#include "SVE/VulkanException.h"
#include "Utils.h"
#include <fstream>

namespace Chewman
{

const std::string soundSettingsFile = "sound.dat";
constexpr uint8_t CurrentSoundSettingsVersion = 1;

GameSoundsManager::GameSoundsManager()
{
    auto* soundSystem = SoundSystem::getInstance();
    if (SoundSystem::getInstance()->isLoaded())
    {
        _soundMap[SoundType::ChewCoin] = soundSystem->createSound("resources/sounds/chew2.ogg");

        soundSystem->initBackgroundMusic("resources/sounds/music.ogg");
        soundSystem->startBackgroundMusic();
    } else {
        _soundEnabled = false;
        _musicEnabled = false;
    }

    load();
}

void GameSoundsManager::playSound(SoundType type)
{
    if (SoundSystem::getInstance()->isLoaded() && _soundEnabled)
        _soundMap.at(type)->play();
}

void GameSoundsManager::setSoundVolume(float value)
{
    _soundVolume = value;
    if (!SoundSystem::getInstance()->isLoaded())
        return;

    for (auto& sound : _soundMap)
        sound.second->setVolume(value);
}

void GameSoundsManager::setMusicVolume(float value)
{
    _musicVolume = value;
    if (SoundSystem::getInstance()->isLoaded())
        SoundSystem::getInstance()->setBackgroundMusicVolume(value);
}

float GameSoundsManager::getSoundVolume() const
{
    return _soundVolume;
}

float GameSoundsManager::getMusicVolume() const
{
    return _musicVolume;
}

void GameSoundsManager::setSoundEnabled(bool value)
{
    _soundEnabled = value;
    if (!SoundSystem::getInstance()->isLoaded())
        _soundEnabled = false;
}

void GameSoundsManager::setMusicEnabled(bool value)
{
    _musicEnabled = value;
    if (!SoundSystem::getInstance()->isLoaded())
        _musicEnabled = false;

    if (_musicEnabled)
        SoundSystem::getInstance()->startBackgroundMusic();
    else
        SoundSystem::getInstance()->stopBackgroundMusic();
}

bool GameSoundsManager::isSoundEnabled() const
{
    return _soundEnabled;
}

bool GameSoundsManager::isMusicEnabled() const
{
    return _musicEnabled;
}

void GameSoundsManager::save()
{
    std::ofstream fout(Utils::getSettingsPath(soundSettingsFile));
    if (!fout)
    {
        throw SVE::VulkanException("Can't save sound settings file");
    }

    fout.write(reinterpret_cast<const char*>(&CurrentSoundSettingsVersion), sizeof(CurrentSoundSettingsVersion));
    fout.write(reinterpret_cast<char*>(&_soundVolume), sizeof(_soundVolume));
    fout.write(reinterpret_cast<char*>(&_musicVolume), sizeof(_musicVolume));
    fout.write(reinterpret_cast<char*>(&_soundEnabled), sizeof(_soundEnabled));
    fout.write(reinterpret_cast<char*>(&_musicEnabled), sizeof(_musicEnabled));
    fout.close();
}

void GameSoundsManager::load()
{
    std::ifstream fin(Utils::getSettingsPath(soundSettingsFile));
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }

    uint8_t version;
    fin.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != CurrentSoundSettingsVersion)
        return;

    fin.read(reinterpret_cast<char*>(&_soundVolume), sizeof(_soundVolume));
    fin.read(reinterpret_cast<char*>(&_musicVolume), sizeof(_musicVolume));
    fin.read(reinterpret_cast<char*>(&_soundEnabled), sizeof(_soundEnabled));
    fin.read(reinterpret_cast<char*>(&_musicEnabled), sizeof(_musicEnabled));

    setMusicVolume(_musicVolume);
    setSoundVolume(_soundVolume);
    setMusicEnabled(_musicEnabled);
    setSoundEnabled(_soundEnabled);
}

} // namespace Chewman