// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"

namespace Chewman
{

class ButtonControl : public Control
{
public:
    ButtonControl(std::string name, float x, float y, float width, float height, Control* parent = nullptr);

    bool onMouseMove(int x, int y) override;
    bool onMouseDown(int x, int y) override;

private:
    static glm::vec4 _defaultPressedColor;
    static glm::vec4 _defaultNormalColor;
    static glm::vec4 _defaultHoverColor;

    glm::vec4 _pressedFontColor = glm::vec4(1);
    glm::vec4 _normalFontColor = glm::vec4(1);
    glm::vec4 _hoverFontColor = glm::vec4(1);
    bool _isFontColorChanged = false;
};

} // namespace Chewman