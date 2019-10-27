// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "DropDownControl.h"
#include "ImageControl.h"

namespace Chewman
{

constexpr float ListElementToControlHeightRatio = 0.5495f;
constexpr float ListTopToControlHeightRatio = 0.2088f;
constexpr float ListBottomToControlHeightRatio = 0.2527f;

DropDownControl::DropDownControl(const std::string& name, float x, float y, float width, float height, Control* parent)
        : Control(ControlType::DropDown, name, x, y, width, height, "dropdown/dropdown.png", parent)
{
    setTextShift({getSize().x / 6, 0});
    setTextAlignment(SVE::TextAlignment::Left);
    setPushMaterial("dropdown/dropdown_pressed.png");

    _top = std::make_shared<ImageControl>(name + "TopImage", 0, 0, 1, ListTopToControlHeightRatio, this);
    _top->setDefaultMaterial("dropdown/list_top.png");
    _top->setVisible(false);
    _top->setRenderOrder(110);
    _bottom = std::make_shared<ImageControl>(name + "BottomImage", 0, 0, 1, ListBottomToControlHeightRatio, this);
    _bottom->setDefaultMaterial("dropdown/list_bottom.png");
    _bottom->setVisible(false);
    _bottom->setRenderOrder(110);

    _children.push_back(_top);
    _children.push_back(_bottom);
}

bool DropDownControl::onMouseUp(int x, int y)
{
    if (isInside(x, y) && _visible)
    {
        if (_listShown)
            _needHide = true;
        else
            showList();
    }
    else
    {
        if (_listShown)
        {
            for (auto& child : _children)
            {
                if (child->isInside(x, y))
                {
                    setText(child->getText());
                    child->onMouseUp(x, y);
                    break;
                }
            }
        }
        _needHide = true;
    }
    return Control::onMouseUp(x, y);
}

void DropDownControl::addChild(std::shared_ptr<Control> control)
{
    Control::addChild(control);
    auto curSize = getSize();

    control->setVisible(false);
    control->setDefaultMaterial("dropdown/list_mid.png");
    control->setHoverMaterial("dropdown/list_mid.png");
    control->setPushMaterial("dropdown/list_mid.png");
    control->setTextShift({curSize.x / 8, 0});
    control->setSize({curSize.x, curSize.y * ListElementToControlHeightRatio});
    control->setRenderOrder(110);
}

void DropDownControl::update(float deltaTime)
{
    Control::update(deltaTime);
    if (_needHide)
    {
        for (auto& child : _children)
            child->setVisible(false);
        _needHide = false;
        _listShown = false;
    }
}

void DropDownControl::setVisible(bool visible)
{
    Control::setVisible(visible);
    _needHide = true;
    _listShown = false;
}

void DropDownControl::showList()
{
    _listShown = true;
    auto parentPos = getPosition();
    auto parentSize = getSize();
    auto currentY = parentPos.y + parentSize.y * 0.85;
    auto elementHeight = parentSize.y * ListElementToControlHeightRatio;

    _children[0]->setPosition({parentPos.x, currentY});
    currentY += parentSize.y * ListTopToControlHeightRatio - 1;
    for (auto i = 2; i < _children.size(); ++i)
    {
        _children[i]->setPosition({parentPos.x, currentY});
        currentY += elementHeight - 1;
    }
    _children[1]->setPosition({parentPos.x, currentY - 1});

    for (auto& child : _children)
        child->setVisible(true);
}

bool DropDownControl::isClickProcessed()
{
    return _listShown && _needHide;
}

} // namespace Chewman