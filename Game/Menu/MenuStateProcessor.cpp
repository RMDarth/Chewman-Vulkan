// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "MenuStateProcessor.h"

namespace Chewman
{

GameState MenuStateProcessor::update(float deltaTime)
{
    return GameState::Level;
}

void MenuStateProcessor::processInput(const SDL_Event& event)
{

}

void MenuStateProcessor::show()
{

}

void MenuStateProcessor::hide()
{

}

bool MenuStateProcessor::isOverlapping()
{
    return false;
}

} // namespace Chewman