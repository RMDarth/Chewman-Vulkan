// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Control.h"

#include <utility>
#include <sstream>
#include "SVE/Engine.h"
#include "SVE/OverlayManager.h"
#include "SVE/FontManager.h"
#include "SVE/MaterialManager.h"

namespace Chewman
{
namespace
{

void formatMessageImpl(std::stringstream& ss)
{
}

template<typename T, typename... Ts>
void formatMessageImpl(std::stringstream& ss, T value, Ts... values)
{
    ss << value;
    formatMessageImpl(ss, values...);
}

template<typename... Ts>
std::string formatMessage(Ts... values)
{
    std::stringstream ss;
    formatMessageImpl(ss, values...);
    return ss.str();
}

} // anon namespace

uint32_t Control::_globalIndex = 0;

Control::Control(ControlType controlType, const std::string& name,
                 float x, float y, float width, float height,
                 const std::string& textureName, Control* parent)
    : _controlType(controlType)
    , _name(name)
    , _index(_globalIndex++)
    , _defaultMaterial(createMaterial(textureName))
    , _parent(parent)
{
    auto* engine = SVE::Engine::getInstance();
    glm::ivec2 parentShift {};
    glm::ivec2 parentSize {};
    if (!parent)
    {
        parentSize = engine->getRenderWindowSize();
    } else {
        parentShift = parent->getPosition();
        parentSize = parent->getSize();
    }
    _x = parentShift.x + parentSize.x * x;
    _y = parentShift.y + parentSize.y * y;
    _width = parentSize.x * width;
    _height = parentSize.y * height;

    if (height < 0)
        _height = (int)(-_width * height);
    if (width < 0)
        _width = (int)(-_height * width);

    SVE::OverlayInfo overlayInfo {};
    overlayInfo.x = _x;
    overlayInfo.y = _y;
    overlayInfo.width = _width;
    overlayInfo.height = _height;
    overlayInfo.name = formatMessage(_name, "_", _index);
    overlayInfo.materialName = _defaultMaterial;
    overlayInfo.textHAlignment = _width > 0 ? SVE::TextAlignment::Center : SVE::TextAlignment::Left;
    _overlay = SVE::Engine::getInstance()->getOverlayManager()->addOverlay(overlayInfo);
}

Control::~Control()
{
    if (auto* overlayManager = SVE::Engine::getInstance()->getOverlayManager())
        overlayManager->removeOverlay(formatMessage(_name, "_", _index));
}

void Control::update(float deltaTime)
{
    for (auto& child : _children)
        child->update(deltaTime);
}

void Control::setDefaultMaterial(const std::string& textureName)
{
    _defaultMaterial = createMaterial(textureName);
    _overlay->setMaterial(_defaultMaterial);
}

void Control::setHoverMaterial(const std::string& textureName)
{
    _hoverMaterial = createMaterial(textureName);
}

void Control::setPushMaterial(const std::string& textureName)
{
    _pushMaterial = createMaterial(textureName);
}

void Control::setDisabledMaterial(const std::string& textureName)
{
    _disabledMaterial = createMaterial(textureName);
}

void Control::setRenderOrder(uint32_t order)
{
    SVE::Engine::getInstance()->getOverlayManager()->changeOverlayOrder(_overlay->getInfo().name, order);
    _overlay->getInfo().zOrder = order;
}

uint32_t Control::getRenderOrder() const
{
    return _overlay->getInfo().zOrder;
}

void Control::setText(const std::string& text, const std::string& font, float scale, glm::vec4 color)
{
    _overlay->setText(SVE::Engine::getInstance()->getFontManager()->generateText(text, font, scale, _textShift, color));
    _text = text;
}

void Control::setText(const std::string& text)
{
    setText(text, _overlay->getInfo().textInfo.font->fontName, _overlay->getInfo().textInfo.scale, _overlay->getInfo().textInfo.color);
}

void Control::setTextShift(glm::ivec2 shift)
{
    _textShift = shift;
}

const std::string& Control::getText() const
{
    return _text;
}

void Control::setTextColor(glm::vec4 color)
{
    _overlay->getInfo().textInfo.color = color;
}

glm::vec4 Control::getTextColor() const
{
    return _overlay->getInfo().textInfo.color;
}


const std::string& Control::getName() const
{
    return _name;
}

ControlType Control::getType() const
{
    return _controlType;
}

void Control::setEnabled(bool enabled)
{
    _enabled = enabled;
}

bool Control::isEnabled() const
{
    return _enabled;
}

void Control::setVisible(bool visible)
{
    _visible = visible;
    _overlay->setVisible(_visible);
}

bool Control::isVisible() const
{
    return _visible;
}

void Control::setMouseTransparent(bool mouseTransparent)
{
    _mouseTransparent = mouseTransparent;
}

bool Control::isMouseTransparent() const
{
    return _mouseTransparent;
}

glm::ivec2 Control::getPosition() const
{
    return glm::ivec2(_x, _y);
}

glm::ivec2 Control::getSize() const
{
    return glm::ivec2(_width, _height);
}

void Control::addChild(std::shared_ptr<Control> control)
{
    _children.push_back(control);
}

void Control::setMouseDownHandler(IEventHandler* handler)
{
    _mouseDownHandlerList.push_back(handler);
}

void Control::setMouseUpHandler(IEventHandler* handler)
{
    _mouseUpHandlerList.push_back(handler);
}

void Control::setMouseMoveHandler(IEventHandler* handler)
{
    _mouseMoveHandlerList.push_back(handler);
}

bool Control::onMouseMove(int x, int y)
{
    if (isInside(x, y) && _visible)
    {
        if (!_hoverMaterial.empty() && !_pressed)
        {
            _overlay->setMaterial(_hoverMaterial);
        }
        if (!_mouseMoveHandlerList.empty())
        {
            std::for_each(_mouseMoveHandlerList.begin(), _mouseMoveHandlerList.end(),
                          [&](IEventHandler* handler) { handler->processEvent(this, IEventHandler::MouseMove, x, y); });
        }

        return true;
    } else {
        if (!_pressed && _visible)
        {
            _overlay->setMaterial(_defaultMaterial);
        }
    }

    return false;
}

bool Control::onMouseDown(int x, int y)
{
    if (isInside(x, y) && _visible)
    {
        if (!_pushMaterial.empty())
        {
            _overlay->setMaterial(_pushMaterial);
        }
        if (!_mouseDownHandlerList.empty())
        {
            std::for_each(_mouseDownHandlerList.begin(), _mouseDownHandlerList.end(),
                          [&](IEventHandler* handler) { handler->processEvent(this, IEventHandler::MouseDown, x, y); });
        }
        _pressed = true;

        if (!_mouseTransparent)
            return true;
    }

    return false;
}

bool Control::onMouseUp(int x, int y)
{
    bool result = false;
    if (isInside(x, y) && _visible)
    {
        _overlay->setMaterial(!_hoverMaterial.empty() ? _hoverMaterial :_defaultMaterial);

        std::for_each(_mouseUpHandlerList.begin(), _mouseUpHandlerList.end(),
                      [&](IEventHandler* handler) { handler->processEvent(this, IEventHandler::MouseUp, x, y); });

        if (!_mouseTransparent)
            result = true;
    } else if (_visible) {
        _overlay->setMaterial(_defaultMaterial);
    }

    _pressed = false;
    return result;
}

bool Control::isInside(int x, int y) const
{
    return x > _x && x < _x + _width
           && y > _y && y < _y + _height;
}

void Control::setCustomAttribute(const std::string& name, std::string value)
{
    if (name == "hoverimage")
    {
        setHoverMaterial(value);
    }
    if (name == "pressedimage")
    {
        setPushMaterial(value);
    }
    if (name == "singleimage" && value == "true")
    {
        _hoverMaterial.clear();
        _pushMaterial.clear();
        _disabledMaterial.clear();
    }
    if (name == "image")
    {
        setDefaultMaterial(value);
    }
    if (name == "order")
    {
        setRenderOrder(std::stoul(value));
    }
    /*if (name == "background-color")
    {
        std::stringstream str(value);
        float color[4];
        str >> color[0] >> color[1] >> color[2] >> color[3];
        setDiffuseColor(color);
    }*/
    if (name == "font-color")
    {
        std::stringstream str(value);
        glm::vec4 color;
        str >> color[0] >> color[1] >> color[2] >> color[3];
        setTextColor(color);
    }
    if (name == "font-size")
    {
        std::stringstream str(value);
        float size;
        str >> size;
        auto* font = _overlay->getInfo().textInfo.font;
        if (font)
            setText(_text, font->fontName, (size / font->size) * 0.5f, _overlay->getInfo().textInfo.color);
    }
}

std::string Control::getCustomAttribute(const std::string& name)
{
    return std::string();
}

std::string Control::createMaterial(const std::string& textureFile)
{
    if (textureFile.empty())
        return std::string();

    std::string name = std::string("ControlTexture_") + textureFile;
    auto* materialManager = SVE::Engine::getInstance()->getMaterialManager();
    auto* material = materialManager->getMaterial(name, true);
    if (!material)
    {
        SVE::MaterialSettings materialSettings{};
        materialSettings.name = name;
        materialSettings.vertexShaderName = "overlayVertexShader";
        materialSettings.fragmentShaderName = "overlayFragmentShader";
        materialSettings.cullFace = SVE::MaterialCullFace::FrontFace;
        materialSettings.useAlphaBlending = true;
        materialSettings.srcBlendFactor = SVE::BlendFactor::SrcAlpha;
        materialSettings.dstBlendFactor = SVE::BlendFactor::OneMinusSrcAlpha;
        materialSettings.useDepthWrite = true;
        materialSettings.useDepthTest = false;

        SVE::TextureInfo textureInfo {};
        textureInfo.samplerName = "texSampler";
        textureInfo.filename = getDefaultOverlayFolder() + textureFile;

        materialSettings.textures.push_back(textureInfo);

        materialManager->registerMaterial(std::make_shared<SVE::Material>(materialSettings));
    }

    return name;
}

std::string Control::getDefaultOverlayFolder()
{
    return "resources/materials/textures/overlay/";
}

void Control::setPosition(glm::ivec2 pos)
{
    _x = pos.x;
    _y = pos.y;
    _overlay->getInfo().x = _x;
    _overlay->getInfo().y = _y;
    if (!_overlay->getInfo().textInfo.text.empty())
        setText(_overlay->getInfo().textInfo.text);
}

void Control::setSize(glm::ivec2 size)
{
    _width = size.x;
    _height = size.y;
    _overlay->getInfo().width = _width;
    _overlay->getInfo().height = _height;
    if (!_overlay->getInfo().textInfo.text.empty())
        setText(_overlay->getInfo().textInfo.text);
}

void Control::setTextAlignment(SVE::TextAlignment alignment)
{
    _overlay->getInfo().textHAlignment = alignment;
    if (!_overlay->getInfo().textInfo.text.empty())
        setText(_overlay->getInfo().textInfo.text);
}

bool Control::isClickProcessed()
{
    return false;
}

} // namespace Chewman