// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Player.h"

#include <SVE/Engine.h>
#include <SVE/SceneManager.h>
#include <SVE/MeshEntity.h>
#include <SVE/LightManager.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Game/GameMap.h"

#include <SDL2/SDL_events.h>

namespace Chewman
{

namespace
{

auto isAntiDirection(MoveDirection curDir, MoveDirection newDir)
{
    return static_cast<uint8_t>(curDir) % 2 == static_cast<uint8_t>(newDir) % 2;
}

std::shared_ptr<SVE::LightNode> addLightEffect(SVE::Engine* engine)
{
    SVE::LightSettings lightSettings {};
    lightSettings.lightType = SVE::LightType::PointLight;
    lightSettings.castShadows = false;
    lightSettings.diffuseStrength = glm::vec4(1.0, 1.0, 0.5, 1.0);
    lightSettings.specularStrength = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    lightSettings.ambientStrength = { 0.3f, 0.3f, 0.3f, 1.0f };
    lightSettings.shininess = 16;
    lightSettings.constAtten = 1.0f * 1.8f;
    lightSettings.linearAtten = 0.35f * 0.15f;
    lightSettings.quadAtten = 0.44f * 0.15f;
    auto lightManager = engine->getSceneManager()->getLightManager();
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings, lightManager->getLightCount());
    lightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 2.5, 0)));
    lightManager->setLight(lightNode, lightManager->getLightCount());

    return lightNode;
}

} // anon namespace

Player::Player(GameMap* gameMap, glm::ivec2 startPos)
    : _mapTraveller(std::make_shared<MapTraveller>(gameMap, startPos))
    , _gameMap(gameMap)
    , _startPos(startPos)
{
    _mapTraveller->setWaterAccessible(true);

    auto* engine = SVE::Engine::getInstance();

    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);

    _rootNode->setNodeTransformation(transform);
    gameMap->mapNode->attachSceneNode(_rootNode);

    auto nunMesh = std::make_shared<SVE::MeshEntity>("trashman");
    nunMesh->setMaterial("Yellow");
    _rootNode->attachEntity(nunMesh);

    _rootNode->attachSceneNode(addLightEffect(engine));

    _cameraAttachNode = engine->getSceneManager()->createSceneNode();
    transform = glm::mat4(1);
    transform = glm::translate(transform, glm::vec3(0, 16.0f, 19.0f));

    _cameraAttachNode->setNodeTransformation(transform);

    _rootNode->attachSceneNode(_cameraAttachNode);
}

void Player::update(float deltaTime)
{
    if (_followMode)
    {
        if (_mapTraveller->isCloseToAffect(MapTraveller::toRealPos(_mapTraveller->getMapPosition())))
        {
            auto mapPos = _mapTraveller->getMapPosition();
            if ( _gameMap->mapData[mapPos.x][mapPos.y].coin)
            {
                _gameMap->mapNode->detachSceneNode(_gameMap->mapData[mapPos.x][mapPos.y].coin->rootNode);
                _gameMap->mapData[mapPos.x][mapPos.y].coin = nullptr;
            }
        }

        updateMovement(deltaTime);
        if (checkForDeath())
            playDeath();

        auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
        camera->setParent(_cameraAttachNode);
        camera->setNodeTransformation(_cameraAttachNode->getTotalTransformation());
        camera->setYawPitchRoll(glm::vec3(0.0f, -(float)atan2(16.0, 19.0), 0.0));
    }

    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);
    auto transform = glm::translate(glm::mat4(1), position);
    const auto rotateAngle = 90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
    transform = glm::rotate(transform, glm::radians(rotateAngle), glm::vec3(0, 1, 0));
    _rootNode->setNodeTransformation(transform);
}

void Player::processInput(const SDL_Event& event)
{
    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_f)
        {
            _followMode = !_followMode;
        }
    }

    if (_followMode)
    {
        const Uint8* keystates = SDL_GetKeyboardState(nullptr);
        if (keystates[SDL_SCANCODE_A])
            _nextMove = MoveDirection::Backward;
        if (keystates[SDL_SCANCODE_D])
            _nextMove = MoveDirection::Forward;
        if (keystates[SDL_SCANCODE_W])
            _nextMove = MoveDirection::Right;
        if (keystates[SDL_SCANCODE_S])
            _nextMove = MoveDirection::Left;
    }
}

void Player::updateMovement(float deltaTime)
{
    if (_mapTraveller->isTargetReached())
    {
        if (_mapTraveller->isMovePossible(_nextMove))
        {
            _mapTraveller->move(_nextMove);
        } else {
            auto current = _mapTraveller->getCurrentDirection();
            if (_mapTraveller->isMovePossible(current))
                _mapTraveller->move(current);
        }
    }
    else if (_nextMove != _mapTraveller->getCurrentDirection() && isAntiDirection(_nextMove, _mapTraveller->getCurrentDirection()))
    {
        if (_mapTraveller->isMovePossible(_nextMove))
            _mapTraveller->move(_nextMove);
    }
    _mapTraveller->update(deltaTime);
}

bool Player::checkForDeath()
{
    auto currentClosestPos = _mapTraveller->getMapPosition();
    auto isCurrentPosAffected = _mapTraveller->isCloseToAffect(MapTraveller::toRealPos(currentClosestPos));

    if (isCurrentPosAffected && _gameMap->mapData[currentClosestPos.x][currentClosestPos.y].cellType == CellType::Liquid)
    {
        return true;
    }

    for (auto& nun : _gameMap->nuns)
    {
        if (_mapTraveller->isCloseToAffect(nun.getPosition()))
        {
            return true;
        }
    }

    return false;
}

void Player::playDeath()
{
    // TODO: Play death animation
    _mapTraveller->setPosition(_startPos);
}

} // namespace Chewman