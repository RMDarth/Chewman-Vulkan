// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <cassert>
#include <cstdint>
#include <fstream>
#include <sstream>
#include "ScoresManager.h"
#include "GameDefs.h"
#include "SVE/Engine.h"
#include "SVE/ResourceManager.h"
#include "SVE/VulkanException.h"
#include "SystemApi.h"

namespace Chewman
{
namespace
{

uint16_t HighscoresFileVersion = 2;

std::string getScoresPath()
{
    auto scoresPath = SVE::Engine::getInstance()->getResourceManager()->getSavePath();
    if (scoresPath.back() != '/' || scoresPath.back() != '\\')
        scoresPath.push_back('/');
    scoresPath += "highscores.dat";
    return scoresPath;
}

} // namespace

ScoresManager::ScoresManager()
{
    load();

    auto scoresSubmitted = System::getIsScoresSubmitted();
    for (int i = 0; i < 36; ++i)
        if (!scoresSubmitted[i])
            _isNew[i] = true;
    if (!scoresSubmitted[36])
    {
        std::cout << "Highscore wasn't submitted" << std::endl;
        _isNewBestScore = true;
    }
}

void ScoresManager::load()
{
    // TODO: Replace it with some encrypted save/load
    _timeScores = std::vector<uint32_t>(LevelsCount, 0);
    _stars = std::vector<uint8_t>(LevelsCount, 0);
    _isNew = std::vector<uint8_t>(LevelsCount, 0);

    std::ifstream fin(getScoresPath());
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }

    //get length of file
    fin.seekg(0, std::ios::end);
    size_t length = fin.tellg();
    fin.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(length);
    fin.read(reinterpret_cast<char*>(data.data()), length);
    fin.close();

    data = System::decryptData(data);
    if (data.empty())
    {
        // decryption error?
        return;
    }
    std::stringstream ss;
    ss.write(reinterpret_cast<char*>(data.data()), data.size());

    auto readValue = [&ss](auto& value) {
        ss.read(reinterpret_cast<char*>(&value), sizeof(value));
    };

    uint16_t version;
    readValue(version);
    if (version != HighscoresFileVersion)
        return; // Add loading prev versions

    uint16_t size;
    readValue(size);
    assert(size <= LevelsCount);
    for (auto i = 0; i < size; ++i)
        readValue(_stars[i]);
    for (auto i = 0; i < size; ++i)
        readValue(_timeScores[i]);
    for (auto i = 0; i < size; ++i)
        readValue(_isNew[i]);
    readValue(_bestScore);
    readValue(_isNewBestScore);
}

void ScoresManager::store()
{
    std::stringstream ss;

    auto writeValue = [&ss](auto value) {
        ss.write(reinterpret_cast<const char*>(&value), sizeof(value));
    };

    writeValue(HighscoresFileVersion);
    assert(_stars.size() == _timeScores.size());
    writeValue((uint16_t)_stars.size());
    for (auto star : _stars)
        writeValue(star);
    for (auto time : _timeScores)
        writeValue(time);
    for (auto isNew : _isNew)
        writeValue(isNew);
    writeValue(_bestScore);
    writeValue(_isNewBestScore);

    auto dataString = ss.str();
    auto* dataPtr = reinterpret_cast<const uint8_t*>(dataString.c_str());
    std::vector<uint8_t> data(dataPtr, dataPtr + dataString.size());
    data = System::encryptData(data);

    std::ofstream fout(getScoresPath());
    if (!fout)
    {
        throw SVE::VulkanException("Can't save highscores file");
    }
    fout.write(reinterpret_cast<char*>(data.data()), data.size());
    fout.close();
}

uint16_t ScoresManager::getStars(uint16_t level) const
{
    assert(level <= _stars.size());
    return _stars[level - 1];
}

uint16_t ScoresManager::getTotalStars() const
{
    uint16_t sum = 0;
    for (auto stars : _stars)
        sum += stars;

    return sum;
}

uint32_t ScoresManager::getTime(uint16_t level) const
{
    assert(level <= _timeScores.size());
    return _timeScores[level - 1];
}

uint32_t ScoresManager::getBestScore() const
{
    return _bestScore;
}

bool ScoresManager::isNewTime(uint16_t level) const
{
    assert(level <= _isNew.size());
    return _isNew[level - 1];
}

bool ScoresManager::isNewBestScore() const
{
    return _isNewBestScore;
}

void ScoresManager::setStars(uint16_t level, uint16_t stars)
{
    assert(level <= _stars.size());
    _stars[level - 1] = static_cast<uint8_t>(stars);
}

void ScoresManager::setTime(uint16_t level, uint32_t time)
{
    assert(level <= _timeScores.size());
    _timeScores[level - 1] = time;
    _isNew[level - 1] = true;
}

void ScoresManager::setBestScore(uint32_t score)
{
    assert(score >= _bestScore);
    _bestScore = score;
    _isNewBestScore = true;
}

void ScoresManager::setIsNewScore(bool value)
{
    _isNewBestScore = value;
}

void ScoresManager::setIsNewTimeScore(uint16_t level, bool value)
{
    assert(level <= _isNew.size());
    _isNew[level - 1] = value;
}

} // namespace Chewman