// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <tinyxml2.h>
#include "ControlDocument.h"
#include "SVE/Engine.h"
#include "SVE/ResourceManager.h"

namespace Chewman
{

ControlDocument::ControlDocument(const std::string& filename)
{
    loadDocument(filename);
}

ControlDocument::~ControlDocument() = default;

void ControlDocument::loadDocument(const std::string& filename)
{
    using namespace tinyxml2;
    XMLDocument doc;

    auto fileContent = SVE::Engine::getInstance()->getResourceManager()->loadFileContent(filename);
    if (doc.Parse(fileContent.data(), fileContent.size()) == XML_SUCCESS)
    {
        XMLElement* element = doc.FirstChildElement("document");

        _rootControl = _controlFactory.createControl(
                "Container",
                "document",
                0.0f, 0.0f, 1.0f, 1.0f, nullptr);

        addChildren(_rootControl, element);
    } else {
        std::cerr << "Can't load control document " << filename << std::endl;
    }
}

void ControlDocument::addChildren(std::shared_ptr<Control> parent, tinyxml2::XMLElement* element)
{
    using namespace tinyxml2;
    std::string alignment;

    float nextX = 0;
    float nextY = 0;
    float padding = 0.05f;

    for (XMLElement* xmlControl = element->FirstChildElement(); xmlControl != nullptr; xmlControl = xmlControl->NextSiblingElement())
    {
        float x;
        float y;

        if (xmlControl->Attribute("padding") != nullptr)
        {
            padding = xmlControl->FloatAttribute("padding");
        }

        if (xmlControl->Attribute("x") != nullptr)
        {
            x = xmlControl->FloatAttribute("x");
        } else {
            x = nextX;
        }

        if (xmlControl->Attribute("y") != nullptr)
        {
            y = xmlControl->FloatAttribute("y");
        } else {
            y = nextY;
        }

        float width = xmlControl->FloatAttribute("width");
        float height = xmlControl->FloatAttribute("height");

        if (width < 0)
        {
            auto parentSize = parent->getSize();
            auto realWidth = -height * parentSize.y * width;
            width = realWidth / parentSize.x;
        }

        nextX += width + padding;

        if (xmlControl->Attribute("alignment") != nullptr)
        {
            alignment = xmlControl->Attribute("alignment");
            if (alignment == "center")
            {
                auto parentSize = parent->getSize();

                x = (((float)parentSize.x * 0.5f) - (width * parentSize.x * 0.5f)) / (float)parentSize.x;

                nextX = 0;
                nextY += height + padding;
            }
            if (alignment == "right")
            {
                auto parentSize = parent->getSize();
                x = ((float)parentSize.x - (width * parentSize.x)) / (float)parentSize.x;

                nextX = 0;
                nextY += height + padding;
            }
        }

        auto gameControl = _controlFactory.createControl(
                xmlControl->Name(),
                xmlControl->Attribute("name"),
                x, y, width, height, parent);

        if (gameControl)
        {
            if (xmlControl->Attribute("textAlignment") != nullptr)
            {
                alignment = xmlControl->Attribute("textAlignment");
                if (alignment == "Left")
                    gameControl->setTextAlignment(SVE::TextAlignment::Left);
                if (alignment == "Right")
                    gameControl->setTextAlignment(SVE::TextAlignment::Right);
                if (alignment == "Center")
                    gameControl->setTextAlignment(SVE::TextAlignment::Center);
            }

            if (xmlControl->Attribute("text") != nullptr)
            {
                float scale = SVE::Engine::getInstance()->getRenderWindowSize().y / 1440.0f;
                gameControl->setText(xmlControl->Attribute("text"), "NordBold", scale, gameControl->getTextColor());
            }

            for (auto* attribute = xmlControl->FirstAttribute(); attribute != nullptr; attribute = attribute->Next())
            {
                gameControl->setCustomAttribute(attribute->Name(), attribute->Value());
            }

            if (xmlControl->Attribute("mousetransparent") != nullptr)
            {
                gameControl->setMouseTransparent(xmlControl->BoolAttribute("mousetransparent"));
            }

            parent->addChild(gameControl);
            _controlList.push_back(gameControl);

            addChildren(gameControl, xmlControl);
        }
    }
}

void ControlDocument::show()
{
    for_each(_controlList.begin(), _controlList.end(),
             [](std::shared_ptr<Control>& control) {  control->setVisible(true); } );
    _visible = true;
}

void ControlDocument::hide()
{
    for_each(_controlList.begin(), _controlList.end(),
             [](std::shared_ptr<Control>& control) {  control->setVisible(false); } );
    _visible = false;
}

bool ControlDocument::isVisible() const
{
    return _visible;
}

void ControlDocument::update(float deltaTime)
{
    for (auto& control : _controlList)
    {
        control->update(deltaTime);
    }
}

void ControlDocument::setMouseDownHandler(IEventHandler* handler)
{
    for_each(_controlList.begin(), _controlList.end(),
             [&](std::shared_ptr<Control>& control) {  control->setMouseDownHandler(handler); } );
}

void ControlDocument::setMouseUpHandler(IEventHandler* handler)
{
    for_each(_controlList.begin(), _controlList.end(),
             [&](std::shared_ptr<Control>& control) {  control->setMouseUpHandler(handler); } );
}

void ControlDocument::setMouseMoveHandler(IEventHandler* handler)
{
    for_each(_controlList.begin(), _controlList.end(),
             [&](std::shared_ptr<Control>& control) {  control->setMouseMoveHandler(handler); } );
}

void ControlDocument::addControl(std::shared_ptr<Control> control)
{
    _controlList.push_back(std::move(control));
}

std::shared_ptr<Control> ControlDocument::getControlByName(const std::string& name)
{
    for (auto& control : _controlList)
    {
        if (control->getName() == name)
        {
            return control;
        }
    }

    return nullptr;
}

bool ControlDocument::onMouseMove(int x, int y)
{
    bool result = false;
    for (auto& control : _controlList)
    {
        result = result | control->onMouseMove(x, y);
    }

    return result;
}

bool ControlDocument::onMouseDown(int x, int y)
{
    bool result = false;
    for (auto& control : _controlList)
    {
        result = result | control->onMouseDown(x, y);
    }

    return result;
}

bool ControlDocument::onMouseUp(int x, int y)
{
    bool result = false;
    for (auto& control : _controlList)
    {
        result = result | control->onMouseUp(x, y);
        if (control->isClickProcessed())
            break;
    }

    return result;
}

void ControlDocument::raisePriority(uint32_t level)
{
    for (auto& control : _controlList)
    {
        control->setRenderOrder(control->getRenderOrder() + level);
    }
}

} // namespace Chewman