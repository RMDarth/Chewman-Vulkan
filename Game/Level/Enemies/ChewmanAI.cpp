// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ChewmanAI.h"
#include "Game/Level/GameMap.h"

namespace Chewman
{

ChewmanAI::ChewmanAI(MapTraveller& mapWalker)
        : RandomWalkerAI(mapWalker)
{
}

void ChewmanAI::update(float deltaTime)
{
    auto pos = _mapTraveller->getRealPosition();
    int enemyIdx = 0;
    for (auto& enemy : _mapTraveller->getGameMap()->enemies)
    {
        if (enemy->isStateActive(EnemyState::Vulnerable))
            continue;

        if (enemy->getMapTraveller().get() != _mapTraveller)
        {
            //std::cout << "Enemy " << enemyIdx << ": " << glm::distance(pos, enemy->getPosition()) << std::endl;
            if (glm::distance(pos, enemy->getPosition()) < 2.5f)
            {
                auto enemyPos = enemy->getMapTraveller()->getMapPosition();
                auto playerPos = _mapTraveller->getMapPosition();
                MoveDirection badDirection;
                if (enemyPos.x < playerPos.x)
                    badDirection = MoveDirection::Left;
                else if (enemyPos.x > playerPos.x)
                    badDirection = MoveDirection::Right;
                else if (enemyPos.y > playerPos.y)
                    badDirection = MoveDirection::Up;
                else
                    badDirection = MoveDirection::Down;

                if (_mapTraveller->isTargetReached())
                {
                    auto getRandomDirection = [this]()
                    {
                        return static_cast<MoveDirection>(std::uniform_int_distribution<>(0, 3)(getRandomEngine()));
                    };

                    MoveDirection direction = getRandomDirection();
                    auto counter = 0;
                    while ((badDirection == direction || !_mapTraveller->tryMove(direction)) && counter <= 4)
                    {
                        direction = static_cast<MoveDirection>((static_cast<uint8_t>(direction) + 1) % 4);
                        counter++;
                    }
                    if (counter > 4)
                    {
                        break;
                    }
                } else {
                    if (_mapTraveller->getCurrentDirection() == badDirection)
                    {
                        MoveDirection dir;
                        for (auto i = 0; i < 4; ++i)
                        {
                            dir = static_cast<MoveDirection>(i);
                            if (isAntiDirection(dir, _mapTraveller->getCurrentDirection()))
                                break;
                        }
                        bool changeTarget = _mapTraveller->getMapPosition() != _mapTraveller->getMapPosition(_mapTraveller->getTargetPos());
                        _mapTraveller->move(dir);
                        if (changeTarget)
                        {
                            _mapTraveller->setTargetPos(MapTraveller::toRealPos(_mapTraveller->getMapPosition()));
                        }

                        if (!_mapTraveller->tryMove(dir))
                        {
                            break;
                        }
                    }
                }

                _mapTraveller->update(deltaTime);
                return;
            }
        }
    }
    RandomWalkerAI::update(deltaTime);
}

void ChewmanAI::setVulnerable(bool value)
{
    _isVulnerable = value;
}

} // namespace Chewman