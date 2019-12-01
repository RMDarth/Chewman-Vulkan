// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "StateProcessor.h"
#include "Game/Controls/ControlDocument.h"

#include <SDL2/SDL_events.h>
#include <sstream>
#include <iomanip>

namespace Chewman
{

void StateProcessor::processDocument(const SDL_Event& event, ControlDocument* controlDocument)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        controlDocument->onMouseMove(event.motion.x, event.motion.y);
    }
    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        controlDocument->onMouseDown(event.button.x, event.button.y);
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {
        controlDocument->onMouseUp(event.button.x, event.button.y);
    }
}

bool StateProcessor::isWideScreen()
{
    auto windowSize = SVE::Engine::getInstance()->getRenderWindowSize();
    if ((float)windowSize.x / windowSize.y < 1.4)
        return false;
    return true;
}

std::string timeToString(uint32_t time)
{
    std::stringstream stream;
    stream << time / 60 << ":" << std::setfill('0') << std::setw(2) << time % 60;
    return stream.str();
}

} // namespace Chewman