// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "GameMap.h"
#include "SVE/Engine.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "Game/GameSoundsManager.h"
#include "Game/Game.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Chewman
{
namespace
{

void updateGargoyleParticles(Gargoyle& gargoyle, float life, float alphaChanger, float lifeDrain)
{
    auto& emitter = gargoyle.particleSystem->getSettings().particleEmitter;
    auto& affector = gargoyle.particleSystem->getSettings().particleAffector;
    emitter.minLife = life;
    emitter.maxLife = life;
    affector.colorChanger = glm::vec4(0.0, 0.0, 0.0, alphaChanger);
    affector.lifeDrain = lifeDrain;
}

} // anon namespace

GameMapProcessor::GameMapProcessor(std::shared_ptr<GameMap> gameMap)
    : _gameRulesProcessor(*this)
    , _gameMap(std::move(gameMap))
{
    SVE::Engine::getInstance()->getSceneManager()->getRootNode()->attachSceneNode(_gameMap->mapNode);
}

GameMapProcessor::~GameMapProcessor()
{
    SVE::Engine::getInstance()->getSceneManager()->getRootNode()->detachSceneNode(_gameMap->mapNode);
    _gameMap.reset();
}

void GameMapProcessor::update(float deltaTime)
{
    _deltaTime = deltaTime;
    if (deltaTime <= 0.0)
        return;

    _gameRulesProcessor.update(deltaTime);

    if (_state == GameMapState::Game)
        _totalTime += deltaTime;
    else
        _deltaTime = 0.0f;

    for (auto& teleport : _gameMap->teleports)
    {
        updateTeleport(_deltaTime, teleport);
    }

    for (auto& coin : _gameMap->coins)
    {
        updateCoin(_deltaTime, coin);
    }

    for (auto& gargoyle : _gameMap->gargoyles)
    {
        updateGargoyle(deltaTime, gargoyle);
    }

    for (auto& enemy : _gameMap->enemies)
    {
        enemy->update(_deltaTime);
    }

    for (auto& powerUp : _gameMap->powerUps)
    {
        powerUp->update(_deltaTime);
    }

    for (auto& staticObject : _gameMap->staticObjects)
    {
        staticObject.update(_deltaTime);
    }

    _gameMap->player->update(_deltaTime);
    _gameMap->eatEffectManager->update(_deltaTime);
}

void GameMapProcessor::setVisible(bool visible)
{
    if (visible == _isVisible)
        return;

    if (!visible)
    {
        SVE::Engine::getInstance()->getSceneManager()->getRootNode()->detachSceneNode(_gameMap->mapNode);
    } else {
        SVE::Engine::getInstance()->getSceneManager()->getRootNode()->attachSceneNode(_gameMap->mapNode);
    }

    _isVisible = visible;
}


void GameMapProcessor::processInput(const SDL_Event& event)
{
    _gameMap->player->processInput(event);
}

void GameMapProcessor::updateGargoyle(float time, Gargoyle& gargoyle)
{
    gargoyle.currentTime += time;
    switch (gargoyle.state)
    {
        case GargoyleState::Fire:
        {
            if (gargoyle.currentTime > gargoyle.fireTime && gargoyle.isFading)
            {
                gargoyle.state = GargoyleState::Rest;
                gargoyle.currentTime = 0;
                gargoyle.currentLength = 0;
                gargoyle.isFading = false;

                if (gargoyle.isFireline)
                {
                    auto& info = gargoyle.fireLine->getInfo();
                    info.percent = 0;
                    info.alpha = 0;
                }
            }
            else if (gargoyle.currentTime > gargoyle.fireTime - 0.2f && !gargoyle.isFading)
            {
                gargoyle.isFading = true;
                if (!gargoyle.isFireline)
                    updateGargoyleParticles(gargoyle, 0, -3.5f, 5.0f);
            }
            else
            {
                float finishPercent = gargoyle.currentTime / gargoyle.fireTime;
                gargoyle.currentLength = (float)gargoyle.lengthInCells * finishPercent;

                if (gargoyle.lightNode)
                {
                    gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::LineLight;
                    gargoyle.lightNode->getLightSettings().secondPoint =
                            gargoyle.startPoint + gargoyle.direction * gargoyle.totalLength * finishPercent;
                }

                if (gargoyle.isFireline)
                {
                    auto& fireLineinfo = gargoyle.fireLine->getInfo();
                    fireLineinfo.percent = finishPercent;
                    fireLineinfo.alpha = glm::clamp((gargoyle.fireTime - gargoyle.currentTime) * 3.5f, 0.3f, 1.0f);
                }
            }
            break;
        }
        case GargoyleState::Rest:
        {
            if (gargoyle.currentTime > 0.2f)
            {
                if (gargoyle.lightNode)
                {
                    gargoyle.lightNode->getLightSettings().lightType = SVE::LightType::None;
                    gargoyle.lightNode->getLightSettings().secondPoint = gargoyle.startPoint;
                }
            }
            if (gargoyle.currentTime > gargoyle.restTime + 0.2f)
            {
                gargoyle.state = GargoyleState::Fire;
                gargoyle.currentTime = 0;
                gargoyle.currentLength = 0;


                if (!gargoyle.isFireline)
                    updateGargoyleParticles(gargoyle, gargoyle.fireTime + gargoyle.restTime, 0.0f, 0.0f);
                else
                    gargoyle.fireLine->getInfo().alpha = 1.0f;

                Game::getInstance()->getSoundsManager().playSound(SoundType::GargoyleStart);
            }
            break;
        }
    }
}

void GameMapProcessor::updateTeleport(float time, Teleport &teleport)
{
    auto updateNode = [](std::shared_ptr<SVE::SceneNode>& node, float time)
    {
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        node->setNodeTransformation(nodeTransform);
    };

    updateNode(teleport.circleNode, _totalTime * 2);
    updateNode(teleport.glowNode, _totalTime * 5);
}

void GameMapProcessor::updateCoin(float time, Coin &coin)
{
    auto transform = coin.rootNode->getNodeTransformation();
    transform = glm::rotate(transform, time, glm::vec3(0.0f, 0.0f, 1.0f));
    coin.rootNode->setNodeTransformation(transform);
}

std::shared_ptr<GameMap> GameMapProcessor::getGameMap()
{
    return _gameMap;
}

void GameMapProcessor::setState(GameMapState gameState)
{
    if (gameState == GameMapState::Pause || gameState == GameMapState::GameOver || gameState == GameMapState::Victory || gameState == GameMapState::LevelStart)
    {
        for (auto& gargoyle : _gameMap->gargoyles)
        {
            if (!gargoyle.isFireline)
                gargoyle.particleSystem->pauseTime();
        }
    } else if (_state == GameMapState::Pause || _state == GameMapState::LevelStart)
    {
        for (auto& gargoyle : _gameMap->gargoyles)
        {
            if (!gargoyle.isFireline)
                gargoyle.particleSystem->unpauseTime();
        }
    } else if (_state == GameMapState::GameOver)
    {
        _gameMap->player->resetPosition();
        _gameMap->player->update(0);
        for (auto& enemy : _gameMap->enemies)
            enemy->resetAll();
    }

    _state = gameState;
    if (_state == GameMapState::LevelStart)
    {
        _gameRulesProcessor.runStartLevelAnimation();
    }
}

GameMapState GameMapProcessor::getState() const
{
    return _state;
}

float GameMapProcessor::getDeltaTime()
{
    return _deltaTime;
}

std::map<PowerUpType, float> GameMapProcessor::getCurrentAffectors() const
{
    return _gameRulesProcessor.getCurrentAffectors();
}

void GameMapProcessor::setNextMove(MoveDirection direction)
{
    _gameMap->player->setNextMove(direction);
}

MoveDirection GameMapProcessor::getNextMove() const
{
    return _gameMap->player->getNextMove();
}

} // namespace Chewman