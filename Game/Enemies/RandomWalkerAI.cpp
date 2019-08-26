// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "RandomWalkerAI.h"

namespace Chewman
{

RandomWalkerAI::RandomWalkerAI(MapTraveller& mapWalker, uint8_t sameWayChance)
    : EnemyAI(mapWalker)
    , _sameWayChange(sameWayChance)
    , _randomDevice(time(0))
{
}

void RandomWalkerAI::update()
{
    if (_mapTraveller->isTargetReached())
    {
        auto getRandomDirection = [this]()
        {
            return static_cast<MoveDirection>(std::uniform_int_distribution<>(0, 3)(_randomDevice));
        };

        auto currentDirection = _mapTraveller->getCurrentDirection();
        if (_mapTraveller->isMovePossible(currentDirection)
            && std::uniform_int_distribution<>(0, 100)(_randomDevice) < _sameWayChange)
        {
            _mapTraveller->move(currentDirection);
        } else {
            auto direction = getRandomDirection();
            while (!_mapTraveller->tryMove(getRandomDirection()))
            {
                direction = static_cast<MoveDirection>((static_cast<uint8_t>(direction) + 1) % 4);
            }
        }
    }

    _mapTraveller->update();
}


} // namespace Chewman