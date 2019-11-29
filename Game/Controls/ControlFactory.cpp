// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ControlFactory.h"
#include "ButtonControl.h"
#include "LabelControl.h"
#include "ImageControl.h"
#include "ContainerControl.h"
#include "PanelControl.h"
#include "BoxSliderControl.h"
#include "LevelButtonControl.h"
#include "DropDownControl.h"

namespace Chewman
{
namespace
{

} // anon namespace

std::shared_ptr<Control> ControlFactory::createControl(
        const std::string& type, const std::string& name,
        float x, float y, float width, float height, std::shared_ptr<Control> parent)
{
    if (type == "Button")
    {
        return std::make_shared<ButtonControl>(name, x, y, width, height, parent.get());
    }
    if (type == "Label")
    {
        return std::make_shared<LabelControl>(name, x, y, width, height, parent.get());
    }
    if (type == "Image")
    {
        return std::make_shared<ImageControl>(name, x, y, width, height, parent.get());
    }
    if (type == "Container")
    {
        return std::make_shared<ContainerControl>(name, x, y, width, height, parent.get());
    }
    if (type == "Panel")
    {
        return std::make_shared<PanelControl>(name, x, y, width, height, parent.get());
    }
    if (type == "BoxSlider")
    {
        return std::make_shared<BoxSliderControl>(name, x, y, width, height, parent.get());
    }
    if (type == "LevelButton")
    {
        return std::make_shared<LevelButtonControl>(name, x, y, width, height, parent.get());
    }
    if (type == "DropDown")
    {
        return std::make_shared<DropDownControl>(name, x, y, width, height, parent.get());
    }

    return nullptr;
}
} // namespace Chewman