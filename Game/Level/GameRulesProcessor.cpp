// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameRulesProcessor.h"
#include "GameMap.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "GameMapLoader.h"

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
    auto& player = gameMap->player;
    auto* playerInfo = player->getPlayerInfo();

    if (playerInfo->isDying)
    {
        playDeath(deltaTime);
        if (_deathTime > 4.7f)
        {
            player->resetPlaying();
            playerInfo->isDying = false;

            if (playerInfo->lives)
            {
                _gameMapProcessor.setState(GameMapState::Game);
                --playerInfo->lives;
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

        // Eat coins and powerUps
        if (mapTraveller->isCloseToAffect(MapTraveller::toRealPos(mapTraveller->getMapPosition())))
        {
            auto mapPos = mapTraveller->getMapPosition();
            if (auto& coin = gameMap->mapData[mapPos.x][mapPos.y].coin)
            {
                gameMap->mapNode->detachSceneNode(coin->rootNode);
                coin = nullptr;
                playerInfo->points += 10;
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
                if (mapTraveller->isCloseToAffect(enemy->getPosition()))
                {
                    if (enemy->isStateActive(EnemyState::Vulnerable))
                        enemy->increaseState(EnemyState::Dead);
                    else
                        return true;
                }
            }

            const auto playerRealPos = glm::vec3(mapTraveller->getRealPosition().y, 0.75, -mapTraveller->getRealPosition().x);
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

        player->getPlayerInfo()->isDying = isPlayerDead;
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
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed);
            break;
        case PowerUpType::Life:
            break;
        case PowerUpType::Bomb:
            break;
        case PowerUpType::Jackhammer:
            break;
        case PowerUpType::Teeth:
            break;
        case PowerUpType::Slow:
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
            _gameAffectors.push_back({type, 10.0f});
            for (auto & enemy : gameMap->enemies)
                enemy->increaseState(EnemyState::Vulnerable);
            break;
        case PowerUpType::Freeze:
            _gameAffectors.push_back({type, 10.0f});
            for (auto & enemy : gameMap->enemies)
                enemy->increaseState(EnemyState::Frozen);
            break;
        case PowerUpType::Acceleration:
            _gameAffectors.push_back({type, 10.0f});
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 2.0f);
            break;
        case PowerUpType::Life:
            break;
        case PowerUpType::Bomb:
        {
            DestroyWalls(pos);
            for (auto& enemy : gameMap->enemies)
                if (glm::length(enemy->getPosition() - getPlayer()->getMapTraveller()->getRealPosition()) < 9.0f)
                    enemy->increaseState(EnemyState::Dead);
            break;
        }
        case PowerUpType::Jackhammer:
            break;
        case PowerUpType::Teeth:
            break;
        case PowerUpType::Slow:
            _gameAffectors.push_back({type, 10.0f});
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 0.5f);
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

void GameRulesProcessor::DestroyWalls(glm::ivec2 pos)
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
    for (auto i = 0; i < 3; ++i)
    {
        gameMap->mapNode->detachEntity(gameMap->mapEntity[i]);
        gameMap->mapEntity[i] = std::make_shared<SVE::MeshEntity>(meshNames[i]);
        gameMap->mapEntity[i]->setRenderToDepth(true);
        gameMap->mapNode->attachEntity(gameMap->mapEntity[i]);
    }
}


} // namespace Chewman