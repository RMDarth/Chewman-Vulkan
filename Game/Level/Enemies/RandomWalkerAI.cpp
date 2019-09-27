// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "RandomWalkerAI.h"
#include "Game/Level/GameUtils.h"

namespace Chewman
{

RandomWalkerAI::RandomWalkerAI(MapTraveller& mapWalker, uint8_t noReturnWayChance)
    : EnemyAI(mapWalker)
    , _noReturnWayChance(noReturnWayChance)
{
}

void RandomWalkerAI::update(float deltaTime)
{
    if (_mapTraveller->isTargetReached())
    {
        auto getRandomDirection = [this]()
        {
            return static_cast<MoveDirection>(std::uniform_int_distribution<>(0, 3)(getRandomEngine()));
        };

        auto currentDirection = _mapTraveller->getCurrentDirection();

        MoveDirection direction;
        do
        {
            direction = getRandomDirection();
            while (isAntiDirection(currentDirection, direction) && std::uniform_int_distribution<>(0, 100)(getRandomEngine()) < _noReturnWayChance)
            {
                direction = static_cast<MoveDirection>((static_cast<uint8_t>(direction) + 1) % 4);
            }
        } while (!_mapTraveller->tryMove(direction));
    }

    _mapTraveller->update(deltaTime);
}


} // namespace Chewman