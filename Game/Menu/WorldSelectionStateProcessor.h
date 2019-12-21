// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Game/StateProcessor.h"
#include "Game/Controls/IEventHandler.h"
#include "Game/ProgressManager.h"
#include <memory>

namespace Chewman
{

class ControlDocument;
class BoxSliderControl;

class WorldSelectionStateProcessor : public StateProcessor, public IEventHandler
{
public:
    WorldSelectionStateProcessor();
    ~WorldSelectionStateProcessor() override;

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

    // IEventHandler
    void processEvent(Control* control, EventType type, int x, int y) override;

private:
    void setLockControlsVisible(bool visible);

    std::unique_ptr<ControlDocument> _document;
    BoxSliderControl* _slider;
    Control* _lockControl;
    uint32_t _currentWorld = 0;
    bool _isLockedLevels = true;
};

} // namespace Chewman