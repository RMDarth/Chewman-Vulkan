// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#include "MapStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Level/GameMapDefs.h"
#include "SVE/SceneManager.h"
#include "SVE/CameraNode.h"
#include "SVE/LightManager.h"

#include <Game/Game.h>
#include <Game/SystemApi.h>

namespace Chewman
{

MapStateProcessor::MapStateProcessor()
        : _document(std::make_unique<ControlDocument>("resources/game/GUI/mapview.xml"))
{
    _document->setMouseUpHandler(this);
    _document->raisePriority(120);
    _document->hide();
}

MapStateProcessor::~MapStateProcessor() = default;

GameState MapStateProcessor::update(float deltaTime)
{
    auto mapSize = Game::getInstance()->getProgressManager().getGameMapService()->getGameMapSize();
    if (_timeToStop > 0)
    {
        auto currentSpeed = _speed * _timeToStop;
        _timeToStop -= deltaTime;
        _pos += currentSpeed * deltaTime;
        _pos = glm::clamp(_pos, glm::vec3(-mapSize.x * CellSize * 0.5f, -25, -mapSize.y * CellSize), glm::vec3(mapSize.x * CellSize * 0.5f, 5, mapSize.y * CellSize * 0.5));

        float k = (_pos.y + 25.0f) / 30.0f;
        float orthoX = -10 - 10.0f * k;
        float orthoX2 = 70.0f + 30.0f * k;
        float orthoZ = 30 + 40.0f * k;
        SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({orthoX, orthoX2, -18.0f, orthoZ}, {5.0f, 200.0f});
    }

    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();
    camera->setPosition({(mapSize.x - 1) * CellSize * 0.5f + _pos.x, 41 + _pos.y, 16 + _pos.z});
    camera->setYawPitchRoll({0.0f, -0.86f, 0});

    return GameState::Map;
}

void MapStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::Pause);
    }

    const auto setSpeed = [&](glm::vec3 speed)
    {
        _timeToStop = 1.0f;
        _speed = speed;
    };

    const Uint8* keystates = SDL_GetKeyboardState(nullptr);
    if (keystates[SDL_SCANCODE_A])
        setSpeed({-15.0f, 0.0f, 0.0f});
    else if (keystates[SDL_SCANCODE_D])
        setSpeed({15.0f, 0.0f, 0.0f});
    else if (keystates[SDL_SCANCODE_W])
        setSpeed({0.0f, 0.0f, -15.0f});
    else if (keystates[SDL_SCANCODE_S])
        setSpeed({0.0f, 0.0f, 15.0f});


    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        _isSliding = true;
        _startSlideX = event.button.x;
        _startSlideY = event.button.y;
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {
        auto windowSize = SVE::Engine::getInstance()->getRenderWindowSize();
        _isSliding = false;
        if (abs(_startSlideX - event.button.x) < windowSize.x * 0.001
            || abs(_startSlideY - event.button.y) < windowSize.y * 0.001)
        {
            return;
        }

        if (abs(event.button.x - _startSlideX) > abs(event.button.y - _startSlideY))
        {
            if (_startSlideX > event.button.x)
                setSpeed({15.0f, 0.0f, 0.0f});
            else
                setSpeed({-15.0f, 0.0f, 0.0f});
        } else {
            if (_startSlideY > event.button.y)
                setSpeed({0.0f, 0.0f, -15.0f});
            else
                setSpeed({0.0f, 0.0f, 15.0f});
        }
    }
}

void MapStateProcessor::show()
{
    _document->show();
    _pos = {};
    _isSliding = false;

    SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({-20.0f, 100.0f, -18.0f, 70.0f}, {5.0f, 200.0f});
}

void MapStateProcessor::hide()
{
    SVE::Engine::getInstance()->getSceneManager()->getLightManager()->setDirectShadowOrtho({-10.0f, 70.0f, -18.0f, 30.0f}, {5.0f, 200.0f});
    SVE::Engine::getInstance()->getSceneManager()->getMainCamera()->setLookAt(glm::vec3(0.0f, 16.0f, 19.0f), glm::vec3(0), glm::vec3(0, 1, 0));
    _document->hide();
}

bool MapStateProcessor::isOverlapping()
{
    return true;
}

void MapStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            hide();
            Game::getInstance()->setState(GameState::Pause);
        }
        if (control->getName() == "zoomin")
        {
            if (_pos.y > -25)
            {
                _timeToStop = 1.0f;
                _speed = glm::vec3(0.0f, -15.0f, -15.0f);
            }
        }
        if (control->getName() == "zoomout")
        {
            if (_pos.y < 5)
            {
                _timeToStop = 1.0f;
                _speed = glm::vec3(0.0f, 15.0f, 15.0f);
            }
        }
    }
}

} // namespace Chewman