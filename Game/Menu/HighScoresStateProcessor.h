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

class HighscoresStateProcessor : public StateProcessor, public IEventHandler
{
public:
    HighscoresStateProcessor();
    ~HighscoresStateProcessor() override;

    GameState update(float deltaTime) override;
    void processInput(const SDL_Event& event) override;

    void show() override;
    void hide() override;

    bool isOverlapping() override;

    // IEventHandler
    void processEvent(Control* control, EventType type, int x, int y) override;

private:
    ControlDocument* getCurrentDoc();
    void updatePointsDoc();
    void updateTimeDoc();
    void updateCheckboxes();
    void updateTimeScores();
    void updatePointScores();

private:
    std::unique_ptr<ControlDocument> _documentPoints;
    std::unique_ptr<ControlDocument> _documentTime;

    bool _isPointsActive = true;
    bool _isWeeklyActive = true;
    bool _isFirstLevelsHalf = true;
};

} // namespace Chewman