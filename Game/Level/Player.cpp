// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Player.h"

#include <SVE/Engine.h>
#include <SVE/SceneManager.h>
#include <SVE/MeshEntity.h>
#include <glm/gtc/matrix_transform.hpp>

#include "GameMap.h"
#include "GameUtils.h"

#include <SDL2/SDL_events.h>

namespace Chewman
{

namespace
{

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
    auto lightNode = std::make_shared<SVE::LightNode>(lightSettings);
    lightNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 2.5, 0)));

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

    _rotateNode = engine->getSceneManager()->createSceneNode();
    _rootNode->attachSceneNode(_rotateNode);

    _trashmanEntity = std::make_shared<SVE::MeshEntity>("trashman");
    _trashmanEntity->setMaterial("Yellow");
    _rotateNode->attachEntity(_trashmanEntity);

    _rootNode->attachSceneNode(addLightEffect(engine));

    createAppearEffect();
    createDisappearEffect();
    createPowerUpEffect();
}

void Player::update(float deltaTime)
{
    if (_followMode)
    {
        if (!_isDying)
        {
            updateMovement(deltaTime);

            if (_isCameraFollow)
            {
                auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
                camera->setParent(_rootNode);
                camera->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
            }

            if (_appearing)
            {
                _appearTime += deltaTime;
                updateAppearEffect();
                if (_appearTime > 0.5f)
                {
                    showAppearEffect(false);
                }
                if (_appearTime > 1.0f)
                {
                    _appearing = false;
                    _trashmanEntity->setMaterial("Yellow");
                    _trashmanEntity->setRenderLast(false);
                    _trashmanEntity->setCastShadows(true);
                }

            }
            if (_powerUpTime > 0)
            {
                _powerUpTime-= deltaTime;

                //const auto rotateAngle = -90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
                //_powerUpEffectNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0)));
            } else {
                _rootNode->detachSceneNode(_powerUpEffectNode);
            }
        }
    }

    const auto realMapPos = _mapTraveller->getRealPosition();
    const auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);
    _rootNode->setNodeTransformation(glm::translate(glm::mat4(1), position));
    const auto rotateAngle = 90.0f * static_cast<uint8_t>(_mapTraveller->getCurrentDirection());
    _rotateNode->setNodeTransformation(glm::rotate(glm::mat4(1), glm::radians(rotateAngle), glm::vec3(0, 1, 0)));
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

std::shared_ptr<MapTraveller> Player::getMapTraveller()
{
    return _mapTraveller;
}

void Player::resetPosition()
{
    _trashmanEntity->setMaterial("YellowAppearTrashman");
    _trashmanEntity->setAnimationState(SVE::AnimationState::Play);
    _trashmanEntity->setRenderLast(true);
    _trashmanEntity->setCastShadows(false);
    _trashmanEntity->resetTime(0.3f);
    _mapTraveller->setPosition(_startPos);
    _nextMove = MoveDirection::None;

    showDisappearEffect(false);
    showAppearEffect(true);
}

void Player::playDeathAnimation()
{
    showDisappearEffect(true);
    _trashmanEntity->setMaterial("YellowBurnTrashman");
    _trashmanEntity->setAnimationState(SVE::AnimationState::Pause);
    _trashmanEntity->setRenderLast();
    _trashmanEntity->setCastShadows(false);
    _trashmanEntity->resetTime();
}

void Player::resetPlaying()
{
    _appearing = true;
    _appearTime = 0.0f;
    _isDying = false;
}

void Player::createAppearEffect()
{
    auto* engine = SVE::Engine::getInstance();
    auto color = glm::vec3(1.0, 1.0, 0.5);

    _appearNode = engine->getSceneManager()->createSceneNode();
    std::shared_ptr<SVE::ParticleSystemEntity> starsPS = std::make_shared<SVE::ParticleSystemEntity>("PowerUp");
    starsPS->getMaterialInfo()->diffuse = glm::vec4(color, 0.6f);
    _appearNode->attachEntity(starsPS);

    _appearNodeGlow = engine->getSceneManager()->createSceneNode();
    _appearNode->attachSceneNode(_appearNodeGlow);
    {
        std::shared_ptr<SVE::MeshEntity> teleportCircleEntity = std::make_shared<SVE::MeshEntity>("cylinder");
        teleportCircleEntity->setMaterial("TeleportCircleMaterial");
        teleportCircleEntity->setRenderLast();
        teleportCircleEntity->setCastShadows(false);
        teleportCircleEntity->getMaterialInfo()->diffuse = { color, 1.0f };
        _appearNodeGlow->attachEntity(teleportCircleEntity);
    }
}

void Player::showAppearEffect(bool show)
{
    if (show)
    {
        _rootNode->attachSceneNode(_appearNode);
    } else {
        _rootNode->detachSceneNode(_appearNode);
    }
}

void Player::showDisappearEffect(bool show)
{
    if (show)
    {
        _rootNode->attachSceneNode(_disappearNode);
    } else {
        _rootNode->detachSceneNode(_disappearNode);
    }
}

void Player::updateAppearEffect()
{
    auto updateNode = [](std::shared_ptr<SVE::SceneNode>& node, float time)
    {
        auto nodeTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        node->setNodeTransformation(nodeTransform);
    };

    updateNode(_appearNodeGlow, _appearTime * 5);
}

void Player::createPowerUpEffect()
{
    auto* engine = SVE::Engine::getInstance();
    auto color = glm::vec3(0.5, 0.5, 1.0);

    _powerUpEffectNode = engine->getSceneManager()->createSceneNode();
    _powerUpPS = std::make_shared<SVE::ParticleSystemEntity>("PowerUp");
    _powerUpPS->getMaterialInfo()->diffuse = glm::vec4(color, 0.6f);
    _powerUpEffectNode->attachEntity(_powerUpPS);

    auto spiralNode = engine->getSceneManager()->createSceneNode();
    spiralNode->setNodeTransformation(glm::translate(glm::mat4(1), glm::vec3(0, 1, 0)));
    _powerUpEffectNode->attachSceneNode(spiralNode);
    _powerUpEntity = std::make_shared<SVE::MeshEntity>("spiral");
    _powerUpEntity->setMaterial("PowerUpMaterial");
    //powerUpNode->setRenderLast();
    _powerUpEntity->setCastShadows(false);
    _powerUpEntity->getMaterialInfo()->diffuse = {1.0, 1.0, 1.0, 1.0f };
    spiralNode->attachEntity(_powerUpEntity);
}

void Player::playPowerUpAnimation()
{
    _powerUpEntity->setMaterial("PowerUpMaterial");
    _powerUpEntity->getMaterialInfo()->diffuse = {1.0, 1.0, 1.0, 1.0f };
    _powerUpPS->getMaterialInfo()->diffuse = glm::vec4(0.5f, 0.5f, 1.0f, 0.6f);
    _rootNode->attachSceneNode(_powerUpEffectNode);
    _powerUpEntity->resetTime();
    _powerUpTime = 1.2f;
}

void Player::playPowerDownAnimation()
{
    _powerUpEntity->setMaterial("PowerDownMaterial");
    _powerUpEntity->getMaterialInfo()->diffuse = {1.0, 0.1, 0.5, 1.0f };
    _powerUpPS->getMaterialInfo()->diffuse = glm::vec4(1.0f, 0.0f, 0.5f, 0.9f);
    _rootNode->attachSceneNode(_powerUpEffectNode);
    _powerUpEntity->resetTime();
    _powerUpTime = 1.2f;
}

void Player::setIsDying(bool isDying)
{
    _isDying = isDying;
}

bool Player::isDying()
{
    return _isDying;
}

void Player::createDisappearEffect()
{
    auto* engine = SVE::Engine::getInstance();
    auto color = glm::vec3(1.0, 0.5, 0.0);

    _disappearNode = engine->getSceneManager()->createSceneNode();
    std::shared_ptr<SVE::ParticleSystemEntity> disappearPS = std::make_shared<SVE::ParticleSystemEntity>("Disappear");
    disappearPS->getMaterialInfo()->diffuse = glm::vec4(color, 1.5f);
    _disappearNode->attachEntity(disappearPS);
}

void Player::setCameraFollow(bool value)
{
    _isCameraFollow = value;
}

} // namespace Chewman