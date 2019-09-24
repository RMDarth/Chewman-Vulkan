// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "GameDefs.h"

union SDL_Event;

namespace Chewman
{

class ControlDocument;

class StateProcessor
{
public:
    StateProcessor() = default;
    virtual ~StateProcessor() = default;

    virtual GameState update(float deltaTime) = 0;
    virtual void processInput(const SDL_Event& event) = 0;

    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isOverlapping() = 0;

    static void processDocument(const SDL_Event& event, ControlDocument* controlDocument);
};

} // namespace Chewman