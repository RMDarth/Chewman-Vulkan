// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{

class SliderControl : public Control
{
public:
    SliderControl(const std::string& name, float x, float y, float width, float height, Control* parent = nullptr);

    void update(float deltaTime) override;

    bool onMouseMove(int x, int y) override;
    bool onMouseDown(int x, int y) override;
    bool onMouseUp(int x, int y) override;

    uint32_t getSelectedObject() const;

private:
    uint32_t _currentObject = 0;
    float _halfWidth = 0;
    float _nextObjShift = 0;

    bool _isMoving = false;
    int _startShift = 0; // displacement on mouse down
    int _currentShift = 0; // displacement on mouse move
    float _remainShift = 0; // drifting to fixed pos after mouse up
};

} // namespace Chewman