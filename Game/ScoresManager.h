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
    uint32_t getTime(uint16_t level) const;
    bool isNewTime(uint16_t level) const;
    uint32_t getBestScore() const;
    bool isNewBestScore() const;
    uint16_t getTotalStars() const;

    void setStars(uint16_t level, uint16_t stars);
    void setTime(uint16_t level, uint32_t time);
    void setBestScore(uint32_t score);
    void setIsNewScore(bool value);
    void setIsNewTimeScore(uint16_t level, bool value);

private:
    std::vector<uint32_t> _timeScores;
    std::vector<uint8_t> _stars;
    std::vector<uint8_t> _isNew;
    uint32_t _bestScore = 0;
    bool _isNewBestScore = false;
};

} // namespace Chewman