// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameRulesProcessor.h"
#include "GameMap.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"

namespace Chewman
{

GameRulesProcessor::GameRulesProcessor(GameMapProcessor& gameMapProcessor)
    : _gameMapProcessor(gameMapProcessor)
{
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
                _gameMapProcessor.setState(GameState::Game);
                --playerInfo->lives;
            }
            else
            {
                // TODO: GameOver
                _gameMapProcessor.setState(GameState::Pause);
            }
        }
    }
    else
    {
        const auto& mapTraveller = player->getMapTraveller();

        // Eat coins
        if (mapTraveller->isCloseToAffect(MapTraveller::toRealPos(mapTraveller->getMapPosition())))
        {
            auto mapPos = mapTraveller->getMapPosition();
            if ( gameMap->mapData[mapPos.x][mapPos.y].coin)
            {
                gameMap->mapNode->detachSceneNode(gameMap->mapData[mapPos.x][mapPos.y].coin->rootNode);
                gameMap->mapData[mapPos.x][mapPos.y].coin = nullptr;
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

            for (auto& nun : gameMap->nuns)
            {
                if (mapTraveller->isCloseToAffect(nun.getPosition()))
                {
                    return true;
                }
            }

            return false;
        }();

        player->getPlayerInfo()->isDying = isPlayerDead;
        if (isPlayerDead)
        {
            _gameMapProcessor.setState(GameState::Animation);
            _deathTime = 0.0f;
            _gameMapProcessor.getGameMap()->player->playDeathAnimation();
            _deathSecondPhase = false;
        }
    }
}

void GameRulesProcessor::playDeath(float deltaTime)
{
    _deathTime += deltaTime;

    auto smoothStep = [](float data)
    {
        return glm::smoothstep(0.0f, 1.0f, data);
    };

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
            auto& player = _gameMapProcessor.getGameMap()->player;
            _cameraStart[0] = camera->getPosition();
            _cameraStart[1] = camera->getYawPitchRoll();

            player->resetPosition();
            player->update(0);
            camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
            _cameraEnd[0] = camera->getPosition();
            _cameraEnd[1] = camera->getYawPitchRoll();
        }

        auto pos = glm::mix(_cameraStart[0], _cameraEnd[0], smoothStep((_deathTime - 3.8f) * 1.1111f));
        auto angles = glm::mix(_cameraStart[1], _cameraEnd[1], smoothStep((_deathTime - 3.8f) * 1.1111f));
        camera->setPosition(pos);
        camera->setYawPitchRoll(angles);
    }
}


} // namespace Chewman