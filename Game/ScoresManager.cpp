// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <cassert>
#include <fstream>
#include "ScoresManager.h"
#include "GameDefs.h"
#include "SVE/Engine.h"
#include "SVE/ResourceManager.h"
#include "SVE/VulkanException.h"

namespace Chewman
{
namespace
{

std::string getScoresPath()
{
    auto scoresPath = SVE::Engine::getInstance()->getResourceManager()->getSavePath();
    if (scoresPath.back() != '/' || scoresPath.back() != '\\')
        scoresPath.push_back('/');
    scoresPath += "scores.dat";
    return scoresPath;
}

} // namespace

ScoresManager::ScoresManager()
{
    load();
}

void ScoresManager::load()
{
    // TODO: Replace it with some encrypted save/load
    _timeScores = std::vector<float>(LevelsCount, -1);
    _stars = std::vector<uint8_t>(LevelsCount, 0);
    std::ifstream fin(getScoresPath());
    if (!fin)
    {
        // Load file doesn't exist
        return;
    }

    auto readValue = [&fin](auto& value) {
        fin.read(reinterpret_cast<char*>(&value), sizeof(value));
    };

    uint16_t size;
    readValue(size);
    assert(size <= LevelsCount);
    for (auto i = 0; i < size; ++i)
        readValue(_stars[i]);
    for (auto i = 0; i < size; ++i)
        readValue(_timeScores[i]);
    readValue(_bestScore);

    fin.close();
}

void ScoresManager::store()
{
    // TODO: Replace it with some encrypted save/load
    std::ofstream fout(getScoresPath());
    if (!fout)
    {
        throw SVE::VulkanException("Can't save scores file");
    }

    auto writeValue = [&fout](auto value) {
        fout.write(reinterpret_cast<const char*>(&value), sizeof(value));
    };

    assert(_stars.size() == _timeScores.size());
    writeValue((uint16_t)_stars.size());
    for (auto star : _stars)
        writeValue(star);
    for (auto time : _timeScores)
        writeValue(time);
    writeValue(_bestScore);

    fout.close();
}

uint16_t ScoresManager::getStars(uint16_t level) const
{
    assert(level <= _stars.size());
    return _stars[level - 1];
}

float ScoresManager::getTime(uint16_t level) const
{
    assert(level <= _timeScores.size());
    return _timeScores[level - 1];
}

uint32_t ScoresManager::getBestScore() const
{
    return _bestScore;
}

void ScoresManager::setStars(uint16_t level, uint16_t stars)
{
    assert(level <= _stars.size());
    _stars[level - 1] = static_cast<uint8_t>(stars);
}

void ScoresManager::setTime(uint16_t level, float time)
{
    assert(level <= _timeScores.size());
    _timeScores[level - 1] = time;
}

void ScoresManager::setBestScore(uint32_t score)
{
    assert(score >= _bestScore);
    _bestScore = score;
}
} // namespace Chewman