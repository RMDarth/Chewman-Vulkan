// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"
#include "ImageControl.h"

namespace Chewman
{

class SliderControl : public Control
{
public:
    SliderControl(const std::string& name, float x, float y, float width, float height, Control* parent = nullptr);

    void setCustomAttribute(const std::string& name, std::string value) override;
    std::string getCustomAttribute(const std::string& name) override;

    void setProgress(float progress);
    void setVisible(bool visible) override;

    bool onMouseMove(int x, int y) override;
    bool onMouseDown(int x, int y) override;
    bool onMouseUp(int x, int y) override;

private:
    float _progress = 0.0f;

    int _progressMaxWidth = 0;
    int _knotStartPos = 0;
    std::shared_ptr<ImageControl> _progressBar;
    std::shared_ptr<ImageControl> _knot;

    bool _slidering = false;
};

} // namespace Chewman