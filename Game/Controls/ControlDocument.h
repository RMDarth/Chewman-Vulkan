// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Control.h"
#include "ControlFactory.h"

namespace tinyxml2
{
class XMLElement;
} // namespace tinyxml2

namespace Chewman
{

class ControlDocument
{
public:
    explicit ControlDocument(const std::string& filename);
    ~ControlDocument();

    void loadDocument(const std::string& filename);

    void show();
    void hide();

    void setMouseDownHandler(IEventHandler* handler);
    void setMouseUpHandler(IEventHandler* handler);
    void setMouseMoveHandler(IEventHandler* handler);

    void addControl(std::shared_ptr<Control> control);
    std::shared_ptr<Control> getControlByName(const std::string& name);

    virtual bool onMouseMove(int x, int y);
    virtual bool onMouseDown(int x, int y);
    virtual bool onMouseUp(int x, int y);

    void raisePriority(uint32_t level);
private:
    void addChildren(std::shared_ptr<Control> parent, tinyxml2::XMLElement* element);

private:
    std::shared_ptr<Control> _rootControl;
    std::vector<std::shared_ptr<Control>> _controlList;

    ControlFactory _controlFactory;
};

} // namespace Chewman