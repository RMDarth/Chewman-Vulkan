// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameRulesProcessor.h"
#include "GameMap.h"
#include "GameMapLoader.h"
#include "Game/Game.h"
#include "Game/Level/Enemies/Projectile.h"
#include "Game/Level/Enemies/Knight.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "SVE/MeshManager.h"
#include "SVE/CameraNode.h"

#include <glm/gtc/matrix_transform.hpp>


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

void GameRulesProcessor::runStartLevelAnimation()
{
    setShadowCamera(false);
    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
    _cameraStart[0] = glm::vec3((_gameMapProcessor.getGameMap()->width - 1) * CellSize * 0.5, 41, 16);
    _cameraStart[1] = glm::vec3(0, -0.86, 0);

    getPlayer()->update(0);
    camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
    _cameraEnd[0] = camera->getPosition();
    _cameraEnd[1] = camera->getYawPitchRoll();

    _cameraSpeed = 0.5f;
    _cameraTime = 0.0f;
    _isCameraMoving = true;
    updateCameraAnimation(0.0f);
    getPlayer()->setCameraFollow(false);
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

template<typename Func>
bool GameRulesProcessor::isGargoyleAffecting(glm::vec3 realPos, const std::shared_ptr<MapTraveller>& mapTraveller, Func func)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    for (auto& gargoyle : gameMap->gargoyles)
    {
        auto toGarg = realPos - gargoyle.startPoint;
        auto projectionDistance = glm::dot(toGarg, gargoyle.direction);
        float finishPercent = gargoyle.currentTime / gargoyle.fireTime;

        if (projectionDistance > 0 && gargoyle.state == GargoyleState::Fire && projectionDistance - CellSize <= gargoyle.totalLength * finishPercent)
        {
            auto projectionPoint = gargoyle.startPoint + gargoyle.direction * projectionDistance;

            if (mapTraveller->isCloseToAffect(glm::vec2(-projectionPoint.z, projectionPoint.x)))
            {
                if (func(gargoyle))
                    return true;
            }
        }
    }

    return false;
}


void GameRulesProcessor::update(float deltaTime)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    auto& player = getPlayer();
    auto& playerInfo = Game::getInstance()->getProgressManager().getPlayerInfo();

    if (player->isDying())
    {
        playDeath(deltaTime);
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

        setShadowCamera(false);
        updateAffectors(deltaTime);

        if (_wallsDownTime > 0.0f)
            updateWallsDown(deltaTime);



        // Eat coins and powerUps
        if (mapTraveller->isCloseToAffect(MapTraveller::toRealPos(mapTraveller->getMapPosition())))
        {
            auto mapPos = mapTraveller->getMapPosition();
            if (eatCoin(mapPos))
            {
                Game::getInstance()->getSoundsManager().playSound(SoundType::ChewCoin);
                gameMap->eatEffectManager->addEffect(EatEffectType::Gold, mapPos);
                playerInfo.points += 10;
                if (gameMap->activeCoins == 0)
                {
                    return;
                }
            }
            if (auto& powerUp = gameMap->mapData[mapPos.x][mapPos.y].powerUp)
            {
                powerUp->eat();
                activatePowerUp(powerUp->getType(), mapPos);
                switch (powerUp->getType())
                {
                    case PowerUpType::Pentagram:
                    case PowerUpType::Freeze:
                    case PowerUpType::Acceleration:
                    case PowerUpType::Life:
                    case PowerUpType::Jackhammer:
                    case PowerUpType::Teeth:
                        Game::getInstance()->getSoundsManager().playSound(SoundType::PowerUp);
                        player->playPowerUpAnimation();
                        break;
                }
                powerUp = nullptr;
            }
            if (auto* teleport = gameMap->mapData[mapPos.x][mapPos.y].teleport)
            {
                if (mapTraveller->isTargetReached() && !_insideTeleport)
                {
                    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
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
            if (gameMap->mapData[mapPos.x][mapPos.y].cellType == CellType::Wall && _activeState[static_cast<uint8_t>(PowerUpType::Teeth)])
            {
                gameMap->mapData[mapPos.x][mapPos.y].cellType = CellType::Floor;
                gameMap->eatEffectManager->addEffect(EatEffectType::Walls, mapPos);
                Game::getInstance()->getSoundsManager().playSound(SoundType::ChewWall);

                auto& node = gameMap->mapData[mapPos.x][mapPos.y].cellBlock;
                auto list = node->getAttachedEntities();
                for (auto &entity : list)
                {
                    gameMap->unusedEntitiesNode->attachEntity(entity);
                }

                node->detachAllEntities();
                updateKnightPath();
            }
        } else {
            _insideTeleport = false;
        }

        for (auto& enemy : gameMap->enemies)
        {
            if (enemy->getEnemyType() == EnemyType::Chewman && !enemy->isStateActive(EnemyState::Dead))
            {
                // eat coins and power ups
                auto enemyTraveller = enemy->getMapTraveller();
                auto enemyPos = enemyTraveller->getMapPosition();
                auto enemyRealPos = MapTraveller::toRealPos(enemyPos);
                if (enemyTraveller->isCloseToAffect(enemyRealPos))
                {
                    eatCoin(enemyPos);
                    if (auto& powerUp = gameMap->mapData[enemyPos.x][enemyPos.y].powerUp)
                    {
                        powerUp->eat();
                        activatePowerUp(powerUp->getType(), enemyPos, enemy.get());
                        switch (powerUp->getType())
                        {
                            case PowerUpType::Pentagram:
                            case PowerUpType::Freeze:
                            case PowerUpType::Acceleration:
                            case PowerUpType::Jackhammer:
                            case PowerUpType::Teeth:
                                player->playPowerUpAnimation();
                                break;
                        }
                        powerUp = nullptr;
                    }
                }

                isGargoyleAffecting(glm::vec3(enemyRealPos.y, 0.75, -enemyRealPos.x), enemyTraveller, [&](Gargoyle& gargoyle)
                {
                    enemy->increaseState(EnemyState::Vulnerable);
                    ++_activeState[static_cast<uint8_t>(PowerUpType::Pentagram)];
                    _gameAffectors.push_back({PowerUpType::Pentagram, PentagrammTotalTime, true});
                    return true;
                });

                // Eat enemies or be eaten
                for (auto& otherEnemy : gameMap->enemies)
                {
                    if (otherEnemy->isStateActive(EnemyState::Dead))
                        continue;

                    if (otherEnemy != enemy && otherEnemy->getMapTraveller()->isCloseToAffect(enemy->getPosition()))
                    {
                        if (otherEnemy->isStateActive(EnemyState::Vulnerable))
                        {
                            otherEnemy->increaseState(EnemyState::Dead);
                        } else {
                            enemy->increaseState(EnemyState::Dead);
                        }
                    }
                }
            }

            if (enemy->getEnemyType() == EnemyType::Knight)
            {
                // collect coins around when in collecting state
                auto* knight = static_cast<Knight*>(enemy.get());
                if (knight->isCollecting())
                {
                    knight->setCollecting(false);
                    auto knightPos = knight->getMapTraveller()->getMapPosition();
                    for (auto x = knightPos.x - 2; x <= knightPos.x + 2; ++x)
                        for (auto y = knightPos.y - 2; y <= knightPos.y + 2; ++y)
                            eatCoin({x, y});
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

            if (mapTraveller->isTargetReached())
            {
                bool isStack = true;
                for (auto direction = 0; direction < 4; ++direction)
                {
                    if (mapTraveller->isMovePossible(static_cast<MoveDirection>(direction)))
                    {
                        isStack = false;
                        break;
                    }
                }
                if (isStack)
                {
                    return true;
                }
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
                    {
                        playerInfo.points += 10;
                        Game::getInstance()->getSoundsManager().playSound(SoundType::ChewEnemy);
                        enemy->increaseState(EnemyState::Dead);
                    }
                    else
                    {
                        enemy->attackPlayer();
                        return true;
                    }
                }
            }

            const auto playerRealPos = glm::vec3(mtRealPos.y, 0.75, -mtRealPos.x);
            if (isGargoyleAffecting(playerRealPos, mapTraveller, [&](Gargoyle& gargoyle)
                {
                    switch(gargoyle.type)
                    {
                        case GargoyleType::Fire:
                            return true;
                        case GargoyleType::Frost:
                            activatePowerUp(PowerUpType::Slow, mapTraveller->getMapPosition());
                    }
                    return false;
                }))
            {
                return true;
            }

            return false;
        }();

        player->setIsDying(isPlayerDead);
        if (isPlayerDead)
        {
            _gameMapProcessor.setState(GameMapState::Animation);
            _deathTime = 0.0f;
            Game::getInstance()->getSoundsManager().playSound(SoundType::Death);
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
        setShadowCamera(true);
        auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
        auto pos = glm::mix(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(-7.0f, 4.0f, 3.0f), smoothStep(_deathTime / 0.5f));
        camera->setLookAt(pos, glm::vec3(0), glm::vec3(0, 1, 0));
    }
    if (_deathTime > 3.8f)
    {
        setShadowCamera(false);
        SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({-10.0f, 70.0f, -18.0f, 30.0f}, {5.0f, 200.0f});
        auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
        if (!_deathSecondPhase)
        {
            _deathSecondPhase = true;
            if (Game::getInstance()->getProgressManager().getPlayerInfo().lives > 0)
            {
                auto& player = getPlayer();

                _cameraStart[0] = camera->getPosition();
                _cameraStart[1] = camera->getYawPitchRoll();

                player->resetPosition();

                for (auto& affector : _gameAffectors)
                    if (affector.powerUp == PowerUpType::Slow || affector.powerUp == PowerUpType::Freeze ||
                        affector.powerUp == PowerUpType::Pentagram)
                        affector.remainingTime = -1;

                player->update(0);
                camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
                _cameraEnd[0] = camera->getPosition();
                _cameraEnd[1] = camera->getYawPitchRoll();

                _cameraSpeed = 1.1111f;
                _cameraTime = 0.0f;
                _isCameraMoving = true;

                auto& enemies = _gameMapProcessor.getGameMap()->enemies;
                for (auto& enemy : enemies)
                    enemy->resetAll();

                updateCameraAnimation(deltaTime);
            }
        } else if (Game::getInstance()->getProgressManager().getPlayerInfo().lives > 0)
        {
            updateCameraAnimation(deltaTime);
        }
    }
    if (_deathTime > 4.7f)
    {
        getPlayer()->resetPlaying();

        auto& playerInfo = Game::getInstance()->getProgressManager().getPlayerInfo();
        if (playerInfo.lives)
        {
            _gameMapProcessor.setState(GameMapState::Game);
            --playerInfo.lives;
        }
        else
        {
            setShadowCamera(false);
            _gameMapProcessor.setState(GameMapState::GameOver);
        }
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
        {
            getPlayer()->getMapTraveller()->setWallAccessible(false);
            auto mapPos = getPlayer()->getMapTraveller()->getMapPosition(getPlayer()->getMapTraveller()->getTargetPos());
            if (gameMap->mapData[mapPos.x][mapPos.y].cellType == CellType::Wall)
            {
                gameMap->mapData[mapPos.x][mapPos.y].cellType = CellType::Floor;
                gameMap->eatEffectManager->addEffect(EatEffectType::Walls, mapPos);
                Game::getInstance()->getSoundsManager().playSound(SoundType::ChewWall);
                regenerateMap();
            }
            break;
        }
        case PowerUpType::Slow:
            if (_lastSpeedPowerUp == type)
                getPlayer()->getMapTraveller()->setSpeed(MoveSpeed);
            break;
    }
}

std::map<PowerUpType, float> GameRulesProcessor::getCurrentAffectors() const
{
    std::map<PowerUpType, float> affectorMap;

    for (const auto& affector : _gameAffectors)
    {
        if (affector.powerUp == PowerUpType::Slow || affector.powerUp == PowerUpType::Acceleration)
        {
            if (_lastSpeedPowerUp != affector.powerUp)
                continue;
        }

        auto iter = affectorMap.find(affector.powerUp);
        if (iter == affectorMap.end())
        {
            affectorMap[affector.powerUp] = affector.remainingTime;
        } else {
            if (iter->second < affector.remainingTime)
            {
                iter->second = affector.remainingTime;
            }
        }
    }

    return affectorMap;
}

void GameRulesProcessor::activatePowerUp(PowerUpType type, glm::ivec2 pos, Enemy* eater)
{
    ++_activeState[static_cast<uint8_t>(type)];
    auto gameMap = _gameMapProcessor.getGameMap();
    switch (type)
    {
        case PowerUpType::Pentagram:
            _gameAffectors.push_back({type, PentagrammTotalTime});
            for (auto & enemy : gameMap->enemies)
            {
                if (enemy.get() != eater)
                    enemy->increaseState(EnemyState::Vulnerable);
            }
            break;
        case PowerUpType::Freeze:
            _gameAffectors.push_back({type, FreezeTotalTime});
            for (auto & enemy : gameMap->enemies)
            {
                if (enemy.get() != eater)
                    enemy->increaseState(EnemyState::Frozen);
            }
            if (eater != nullptr)
            {
                ++_activeState[static_cast<uint8_t>(PowerUpType::Slow)];
                _gameAffectors.push_back({PowerUpType::Slow, SlowTotalTime});
                getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 0.5f);
                _activeState[static_cast<uint8_t>(PowerUpType::Acceleration)] = 0;
                _lastSpeedPowerUp = PowerUpType::Slow;
            }
            break;
        case PowerUpType::Acceleration:
            _gameAffectors.push_back({type, AccelerationTotalTime});
            getPlayer()->getMapTraveller()->setSpeed(MoveSpeed * 2.0f);
            _activeState[static_cast<uint8_t>(PowerUpType::Slow)] = 0;
            _lastSpeedPowerUp = type;
            break;
        case PowerUpType::Life:
            if (eater == nullptr)
                Game::getInstance()->getProgressManager().getPlayerInfo().lives += 2;
            break;
        case PowerUpType::Bomb:
        {
            destroyWalls(pos);
            Game::getInstance()->getSoundsManager().playSound(SoundType::Bomb);
            for (auto& enemy : gameMap->enemies)
            {
                if (glm::length(enemy->getPosition() - getPlayer()->getMapTraveller()->getRealPosition()) < 9.0f)
                    enemy->increaseState(EnemyState::Dead);
            }
            break;
        }
        case PowerUpType::Jackhammer:
            _gameAffectors.push_back({type, JackhammerTotalTime});
            _wallsDownTime = JackhammerTotalTime;
            getPlayer()->getMapTraveller()->setWallAccessible(true);
            break;
        case PowerUpType::Teeth:
            _gameAffectors.push_back({type, TeethTotalTime});
            getPlayer()->getMapTraveller()->setWallAccessible(true);
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

                auto& node = gameMap->mapData[x][y].cellBlock;
                auto list = node->getAttachedEntities();
                for (auto& entity : list)
                {
                    gameMap->unusedEntitiesNode->attachEntity(entity);
                }
                node->detachAllEntities();
            }
        }
    }

    updateKnightPath();
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

void GameRulesProcessor::updateKnightPath()
{
    auto gameMap = _gameMapProcessor.getGameMap();
    if (std::any_of(gameMap->enemies.begin(), gameMap->enemies.end(),
                    [](std::unique_ptr<Enemy>& enemy) { return enemy->getEnemyType() == EnemyType::Knight; }))
    {
        Knight::updatePathMap(gameMap.get());
    }
}

bool GameRulesProcessor::eatCoin(glm::ivec2 pos)
{
    auto gameMap = _gameMapProcessor.getGameMap();
    if (pos.x < 0 || pos.x >= gameMap->height ||
        pos.y < 0 || pos.y >= gameMap->width)
        return false;
    if (auto& coin = gameMap->mapData[pos.x][pos.y].coin)
    {
        gameMap->mapNode->detachSceneNode(coin->rootNode);
        coin = nullptr;
        --gameMap->activeCoins;
        if (gameMap->activeCoins == 0)
        {
            std::cout << "You won!" << std::endl;
            Game::getInstance()->getSoundsManager().playSound(SoundType::Victory);
            _gameMapProcessor.setState(GameMapState::Victory);
        }
        return true;
    }
    return false;
}

void GameRulesProcessor::setShadowCamera(bool isZoomed)
{
    if (isZoomed)
        SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({-20.0f, 20.0f, -30.0f, 30.0f}, {5.0f, 200.0f});
    else
        SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({-10.0f, 70.0f, -18.0f, 30.0f}, {5.0f, 200.0f});
}

void GameRulesProcessor::resetLevel()
{
    auto player = getPlayer();
    player->resetPosition();
    player->update(0);

    auto& enemies = _gameMapProcessor.getGameMap()->enemies;
    for (auto& enemy : enemies)
        enemy->resetAll();
}

} // namespace Chewman