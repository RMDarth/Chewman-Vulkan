// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameRulesProcessor.h"
#include "GameMap.h"
#include "GameMapLoader.h"
#include "Game/Game.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"

#include <glm/gtc/matrix_transform.hpp>
#include <Game/Level/Enemies/Projectile.h>

namespace Chewman
{

namespace
{

inline float smoothStep(float data)
{
    return glm::smoothstep(0.0f, 1.0f, data);
}

} // anon namespace

GameRulesProcessor::GameRulesProcessor(GameMapProcessor& gameMapProcessor)
    : _gameMapProcessor(gameMapProcessor)
{
}

std::shared_ptr<Player>& GameRulesProcessor::getPlayer()
{
    if (!_player)
    {
        auto gameMap = _gameMapProcessor.getGameMap();
        _player = gameMap->player;
    }
    return _player;
}

void GameRulesProcessor::update(float deltaTime)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    auto& player = getPlayer();
    auto& playerInfo = Game::getInstance()->getProgressManager().getPlayerInfo();

    if (player->isDying())
    {
        playDeath(deltaTime);
        if (_deathTime > 4.7f)
        {
            player->resetPlaying();

            if (playerInfo.lives)
            {
                _gameMapProcessor.setState(GameMapState::Game);
                --playerInfo.lives;
            }
            else
            {
                _gameMapProcessor.setState(GameMapState::GameOver);
            }
        }
    }
    else
    {
        const auto& mapTraveller = player->getMapTraveller();

        if (_isCameraMoving)
        {
            updateCameraAnimation(deltaTime);
            if (!_isCameraMoving)
            {
                _gameMapProcessor.setState(GameMapState::Game);
                player->setCameraFollow(true);
            }
            else
            {
                return;
            }
        }

        updateAffectors(deltaTime);

        if (_wallsDownTime > 0.0f)
            updateWallsDown(deltaTime);

        // Eat coins and powerUps
        if (mapTraveller->isCloseToAffect(MapTraveller::toRealPos(mapTraveller->getMapPosition())))
        {
            auto mapPos = mapTraveller->getMapPosition();
            if (auto& coin = gameMap->mapData[mapPos.x][mapPos.y].coin)
            {
                gameMap->mapNode->detachSceneNode(coin->rootNode);
                coin = nullptr;
                playerInfo.points += 10;
                --gameMap->activeCoins;
                if (gameMap->activeCoins == 0)
                {
                    std::cout << "You won!" << std::endl;
                    _gameMapProcessor.setState(GameMapState::Victory);
                }
            }
            if (auto& powerUp = gameMap->mapData[mapPos.x][mapPos.y].powerUp)
            {
                powerUp->eat();
                activatePowerUp(powerUp->getType(), mapPos);
                player->playPowerUpAnimation();
                powerUp = nullptr;
            }
            if (auto* teleport = gameMap->mapData[mapPos.x][mapPos.y].teleport)
            {
                if (mapTraveller->isTargetReached() && !_insideTeleport)
                {
                    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
                    auto& player = getPlayer();
                    _cameraStart[0] = camera->getPosition();
                    _cameraStart[1] = camera->getYawPitchRoll();

                    mapTraveller->setPosition(teleport->secondEnd->position);
                    player->update(0);
                    camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
                    _cameraEnd[0] = camera->getPosition();
                    _cameraEnd[1] = camera->getYawPitchRoll();

                    _cameraSpeed = 1.5f;
                    _cameraTime = 0.0f;
                    _isCameraMoving = true;
                    _gameMapProcessor.setState(GameMapState::Animation);
                    updateCameraAnimation(0.0f);
                    player->setCameraFollow(false);
                    _insideTeleport = true;
                    return;
                }
            }
        } else {
            _insideTeleport = false;
        }

        for (auto& enemy : gameMap->enemies)
        {
            if (enemy->getEnemyType() == EnemyType::Chewman)
            {
                auto enemyTraveller = enemy->getMapTraveller();
                auto enemyPos = enemyTraveller->getMapPosition();
                if (enemyTraveller->isCloseToAffect(MapTraveller::toRealPos(enemyPos)))
                {
                    if (auto& coin = gameMap->mapData[enemyPos.x][enemyPos.y].coin)
                    {
                        gameMap->mapNode->detachSceneNode(coin->rootNode);
                        coin = nullptr;
                        --gameMap->activeCoins;
                        if (gameMap->activeCoins == 0)
                        {
                            std::cout << "You won!" << std::endl;
                            _gameMapProcessor.setState(GameMapState::Victory);
                        }
                    }
                    if (auto& powerUp = gameMap->mapData[enemyPos.x][enemyPos.y].powerUp)
                    {
                        powerUp->eat();
                        activatePowerUp(powerUp->getType(), enemyPos);
                        player->playPowerUpAnimation();
                        powerUp = nullptr;
                    }
                }
            }
        }


        // Check death
        bool isPlayerDead = [&]() {
            auto mtRealPos = mapTraveller->getRealPosition();
            auto currentClosestPos = mapTraveller->getMapPosition();
            auto isCurrentPosAffected = mapTraveller->isCloseToAffect(MapTraveller::toRealPos(currentClosestPos));

            if (isCurrentPosAffected &&
                gameMap->mapData[currentClosestPos.x][currentClosestPos.y].cellType == CellType::Liquid)
            {
                return true;
            }

            for (auto& enemy : gameMap->enemies)
            {
                if (enemy->isDead())
                    continue;
                if (enemy->getMapTraveller()->isCloseToAffect(mtRealPos))
                {
                    if (enemy->getEnemyType() == EnemyType::Projectile
                        && static_cast<Projectile*>(enemy.get())->getProjectileType() == ProjectileType::Frost)
                    {
                        activatePowerUp(PowerUpType::Slow, mapTraveller->getMapPosition());
                        enemy->increaseState(EnemyState::Dead);
                    }
                    else if (enemy->isStateActive(EnemyState::Vulnerable))
                        enemy->increaseState(EnemyState::Dead);
                    else
                        return true;
                }
            }

            const auto playerRealPos = glm::vec3(mtRealPos.y, 0.75, -mtRealPos.x);
            for (auto& gargoyle : gameMap->gargoyles)
            {
                auto toGarg = playerRealPos - gargoyle.startPoint;
                auto projectionDistance = glm::dot(toGarg, gargoyle.direction);
                float finishPercent = gargoyle.currentTime / gargoyle.fireTime;

                if (projectionDistance > 0 && gargoyle.state == GargoyleState::Fire && projectionDistance - CellSize <= gargoyle.totalLength * finishPercent)
                {
                    auto projectionPoint = gargoyle.startPoint + gargoyle.direction * projectionDistance;

                    if (mapTraveller->isCloseToAffect(glm::vec2(-projectionPoint.z, projectionPoint.x)))
                    {
                        switch(gargoyle.type)
                        {
                            case GargoyleType::Fire:
                                return true;
                                break;
                            case GargoyleType::Frost:
                                activatePowerUp(PowerUpType::Slow, mapTraveller->getMapPosition());
                                break;
                        }
                    }
                }
            }

            return false;
        }();

        player->setIsDying(isPlayerDead);
        if (isPlayerDead)
        {
            _gameMapProcessor.setState(GameMapState::Animation);
            _deathTime = 0.0f;
            player->playDeathAnimation();
            _deathSecondPhase = false;
        }
    }
}

void GameRulesProcessor::playDeath(float deltaTime)
{
    _deathTime += deltaTime;

    if (_deathTime <= 0.5f)
    {
        auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
        auto pos = glm::mix(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(-7.0f, 4.0f, 3.0f), smoothStep(_deathTime / 0.5f));
        camera->setLookAt(pos, glm::vec3(0), glm::vec3(0, 1, 0));
    }
    if (_deathTime > 3.8f)
    {
        auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
        if (!_deathSecondPhase)
        {
            _deathSecondPhase = true;
            auto& player = getPlayer();
            _cameraStart[0] = camera->getPosition();
            _cameraStart[1] = camera->getYawPitchRoll();

            player->resetPosition();
            player->update(0);
            camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
            _cameraEnd[0] = camera->getPosition();
            _cameraEnd[1] = camera->getYawPitchRoll();

            _cameraSpeed = 1.1111f;
            _cameraTime = 0.0f;
            _isCameraMoving = true;
        }

        updateCameraAnimation(deltaTime);
    }
}

void GameRulesProcessor::updateAffectors(float deltaTime)
{
    for (auto& affector : _gameAffectors)
    {
        affector.remainingTime -= deltaTime;
        if (affector.remainingTime < 1.0 && !affector.isPoweredDown)
        {
            getPlayer()->playPowerDownAnimation();
            affector.isPoweredDown = true;
        }
        if (affector.remainingTime <= 0)
        {
            deactivatePowerUp(affector.powerUp);
        }
    }

    _gameAffectors.erase(
            std::remove_if(_gameAffectors.begin(), _gameAffectors.end(),
                    [](const GameAffector& affector) { return affector.remainingTime <= 0.0f; } ),
            _gameAffectors.end());

}

void GameRulesProcessor::deactivatePowerUp(PowerUpType type)
{
    auto typeIndex = static_cast<uint8_t>(type);
    if (_activeState[typeIndex] > 0)
        --_activeState[typeIndex];
    if (_activeState[typeIndex])
        return;

    auto gameMap = _gameMapProcessor.getGameMap();

    switch (type)
    {
        case PowerUpType::Pentagram:
            for (auto & enemy : gameMap->enemies)
                enemy->resetState(EnemyState::Vulnerable);
            break;
        case PowerUpType::Freeze:
            for (auto & enemy : gameMap->enemies)
                enemy->resetState(EnemyState::Frozen);
            break;
        case PowerUpType::Acceleration:
            if (_lastSpeedPowerUp == type)
                getPlayer()->getMapTraveller()->setSpeed(MoveSpeed);
            break;
        case PowerUpType::Life:
            break;
        case PowerUpType::Bomb:
            break;
        case PowerUpType::Jackhammer:
            getPlayer()->getMapTraveller()->setWallAccessible(false);
            break;
        case PowerUpType::Teeth:
            break;
        case PowerUpType::Slow:
            if (_lastSpeedPowerUp == type)
                getPlayer()->getMapTraveller()->setSpeed(MoveSpeed);
            break;
    }
}

void GameRulesProcessor::activatePowerUp(PowerUpType type, glm::ivec2 pos)
{
    ++_activeState[static_cast<uint8_t>(type)];
    auto gameMap = _gameMapProcessor.getGameMap();
    switch (type)
    {
        case PowerUpType::Pentagram:
            _gameAffectors.push_back({type, PentagrammTotalTime});
            for (auto & enemy : gameMap->enemies)
                enemy->increaseState(EnemyState::Vulnerable);
            break;
        case PowerUpType::Freeze:
            _gameAffectors.push_back({type, FreezeTotalTime});
            for (auto & enemy : gameMap->enemies)
                enemy->increaseState(EnemyState::Frozen);
            break;
        case PowerUpType::Acceleration:
            _gameAffectors.push_back({type, AccelerationTotalTime});
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 2.0f);
            _activeState[static_cast<uint8_t>(PowerUpType::Slow)] = 0;
            _lastSpeedPowerUp = type;
            break;
        case PowerUpType::Life:
            Game::getInstance()->getProgressManager().getPlayerInfo().lives += 2;
            break;
        case PowerUpType::Bomb:
        {
            destroyWalls(pos);
            for (auto& enemy : gameMap->enemies)
                if (glm::length(enemy->getPosition() - getPlayer()->getMapTraveller()->getRealPosition()) < 9.0f)
                    enemy->increaseState(EnemyState::Dead);
            break;
        }
        case PowerUpType::Jackhammer:
            _gameAffectors.push_back({type, JackhammerTotalTime});
            _wallsDownTime = JackhammerTotalTime;
            getPlayer()->getMapTraveller()->setWallAccessible(true);
            break;
        case PowerUpType::Teeth:
            break;
        case PowerUpType::Slow:
            _gameAffectors.push_back({type, SlowTotalTime});
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 0.5f);
            _activeState[static_cast<uint8_t>(PowerUpType::Acceleration)] = 0;
            _lastSpeedPowerUp = type;
            break;
    }
}

void GameRulesProcessor::updateCameraAnimation(float deltaTime)
{
    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();

    _cameraTime += deltaTime * _cameraSpeed;
    auto pos = glm::mix(_cameraStart[0], _cameraEnd[0], smoothStep(_cameraTime));
    auto angles = glm::mix(_cameraStart[1], _cameraEnd[1], smoothStep(_cameraTime));
    camera->setPosition(pos);
    camera->setYawPitchRoll(angles);

    if (_cameraTime >= 1.0f)
    {
        _isCameraMoving = false;
    }
}

void GameRulesProcessor::destroyWalls(glm::ivec2 pos)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    for (auto x = pos.x - 2; x <= pos.x + 2; x++)
    {
        for (auto y = pos.y - 2; y <= pos.y + 2; y++)
        {
            if (x < 0 || y < 0 || x >= gameMap->height || y >= gameMap->width)
                continue;

            if (gameMap->mapData[x][y].cellType == CellType::Wall)
            {
                gameMap->mapData[x][y].cellType = CellType::Floor;
            }
        }
    }

    BlockMeshGenerator blockMeshGenerator(CellSize);
    buildLevelMeshes(*gameMap, blockMeshGenerator);

    std::string meshNames[] = { "MapT", "MapB", "MapV" };
    std::shared_ptr<SVE::SceneNode> nodes[] = {
            gameMap->upperLevelMeshNode,
            gameMap->mapNode,
            gameMap->upperLevelMeshNode
    };
    for (auto i = 0; i < 3; ++i)
    {
        nodes[i]->detachEntity(gameMap->mapEntity[i]);
        gameMap->mapEntity[i] = std::make_shared<SVE::MeshEntity>(meshNames[i]);
        gameMap->mapEntity[i]->setRenderToDepth(true);
        nodes[i]->attachEntity(gameMap->mapEntity[i]);
    }
}

void GameRulesProcessor::updateWallsDown(float deltaTime)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    _wallsDownTime -= deltaTime;

    float scale = 0.05f;
    if (_wallsDownTime + 0.5f > JackhammerTotalTime)
    {
        auto delta = JackhammerTotalTime - _wallsDownTime; // 0...0.5
        scale = std::max(1.0f - delta * 2, 0.05f);
    }
    else if (_wallsDownTime < 0.5f)
    {
        scale = std::min(1.0f - _wallsDownTime * 2, 1.0f);
    }
    gameMap->upperLevelMeshNode->setNodeTransformation(glm::scale(glm::mat4(1), glm::vec3(1.0f, scale, 1.0)));
}


} // namespace Chewman