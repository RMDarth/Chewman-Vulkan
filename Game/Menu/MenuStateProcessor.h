// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Game/StateProcessor.h"
#include "Game/Controls/IEventHandler.h"
#include <memory>

namespace Chewman
{
class ControlDocument;
class GameMapProcessor;

class MenuStateProcessor : public StateProcessor, public IEventHandler
{
public:
    MenuStateProcessor();
    ~MenuStateProcessor() override;

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

    // IEventHandler
    void processEvent(Control* control, EventType type, int x, int y) override;

private:
    void updateConfigSlider();
    void setConfigSliderVisibility(bool visible);
    void updateSoundButtons();

private:
    std::unique_ptr<ControlDocument> _document;
    std::unique_ptr<GameMapProcessor> _gameMapProcessor;
    std::shared_ptr<Control> _configPanel;
    bool _configPanelVisible = false;
};

} // namespace Chewman