// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <vector>
#include <cstdint>

namespace Chewman
{

class ScoresManager
{
public:
    ScoresManager();
    ScoresManager(const ScoresManager&) = delete;

    void load();
    void store();

    uint16_t getStars(uint16_t level) const;
    float getTime(uint16_t level) const;
    uint32_t getBestScore() const;

    void setStars(uint16_t level, uint16_t stars);
    void setTime(uint16_t level, float time);
    void setBestScore(uint32_t score);

private:
    std::vector<float> _timeScores;
    std::vector<uint8_t> _stars;
    uint32_t _bestScore = 0;
};

} // namespace Chewman