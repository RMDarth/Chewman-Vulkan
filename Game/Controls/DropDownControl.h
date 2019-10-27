// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{
class ImageControl;

class DropDownControl : public Control
{
public:
    DropDownControl(const std::string& name, float x, float y, float width, float height, Control* parent = nullptr);

    bool onMouseUp(int x, int y) override;
    void addChild(std::shared_ptr<Control> control) override;

    void update(float deltaTime) override;
    void setVisible(bool visible) override;

    bool isClickProcessed() override;

private:
    void showList();

    bool _needHide = false;
    bool _listShown = false;
    std::shared_ptr<ImageControl> _top;
    std::shared_ptr<ImageControl> _bottom;


};

} // namespace Chewman