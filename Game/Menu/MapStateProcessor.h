// Chewman Vulkan game
// Copyright (c) 2018-2020, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Game/StateProcessor.h"
#include "Game/Controls/IEventHandler.h"
#include <glm/vec3.hpp>
#include <memory>

namespace Chewman
{

class ControlDocument;

class MapStateProcessor : public StateProcessor, public IEventHandler
{
public:
    MapStateProcessor();
    ~MapStateProcessor() override;

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

    // IEventHandler
    void processEvent(Control* control, EventType type, int x, int y) override;

private:
    glm::vec3 _speed;
    glm::vec3 _pos;
    float _timeToStop;

    bool _isSliding = false;
    float _startSlideX = 0;
    float _startSlideY = 0;

    std::unique_ptr<ControlDocument> _document;
};

} // namespace Chewman