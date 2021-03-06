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

class ScoreStateProcessor : public StateProcessor, public IEventHandler
{
public:
    ScoreStateProcessor();
    ~ScoreStateProcessor() override;

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

    // IEventHandler
    void processEvent(Control* control, EventType type, int x, int y) override;

private:
    std::unique_ptr<ControlDocument> _document;
    ProgressManager& _progressManager;
    int _restartX;
    float _time = 0;
    float _startStars[3] = {};
    uint16_t _stars = 0;
    uint16_t _countStars = 0;
    bool _countingFinished = false;

    std::string _scoreLabel;
    std::string _timeLabel;
};

} // namespace Chewman