// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "IEventHandler.h"
#include "SVE/OverlayEntity.h"

// Control system based on my control system for Ogre3D engine from my older games (Bubble Shooter 3D, Space Rush 3D)
// Uses SVE Overlay system instead of Ogre, some parts are refactored.

namespace Chewman
{

enum class ControlType
{
    Button,
    Image,
    Label,
    Container,
    Panel,
    BoxSlider,
    LevelButton,
    DropDown,
    Slider
};

class Control
{
public:
    Control(ControlType controlType,
            const std::string& name,
            float x,
            float y,
            float width,
            float height,
            const std::string& defaultTextureName,
            Control* parent = nullptr);
    virtual ~Control();

    virtual void update(float deltaTime);
    virtual void setDefaultMaterial(const std::string& textureName);
    virtual void setHoverMaterial(const std::string& textureName);
    virtual void setPushMaterial(const std::string& textureName);
    virtual void setDisabledMaterial(const std::string& textureName);
    virtual void setRawMaterial(const std::string& materialName);

    virtual void setRenderOrder(uint32_t order);
    virtual uint32_t getRenderOrder() const;
    virtual void setText(const std::string& text, const std::string& font, float scale = 1.0f, glm::vec4 color = {1,1,1,1});
    virtual void setText(const std::string& text);
    virtual void setTextShift(glm::ivec2 shift);
    virtual void setTextAlignment(SVE::TextAlignment alignment);
    virtual const std::string& getText() const;
    virtual void setTextColor(glm::vec4 color);
    virtual glm::vec4 getTextColor() const;

    virtual const std::string& getName() const;
    virtual ControlType getType() const;

    virtual void setEnabled(bool enabled);
    virtual bool isEnabled() const;

    virtual void setVisible(bool visible);
    virtual bool isVisible() const;

    virtual void setMouseTransparent(bool mouseTransparent);
    virtual bool isMouseTransparent() const;
    virtual bool isClickProcessed();

    virtual glm::ivec2 getPosition() const;
    virtual glm::ivec2 getSize() const;
    virtual void setPosition(glm::ivec2 pos);
    virtual void setSize(glm::ivec2 size);
    virtual void setTexCoords(glm::vec4 xxyy);

    virtual void setCustomAttribute(const std::string& name, std::string value);
    virtual std::string getCustomAttribute(const std::string& name);

    virtual void addChild(std::shared_ptr<Control> control);
    // TODO: Add children accessors

    virtual void setMouseDownHandler(IEventHandler* handler);
    virtual void setMouseUpHandler(IEventHandler* handler);
    virtual void setMouseMoveHandler(IEventHandler* handler);

    virtual bool onMouseMove(int x, int y);
    virtual bool onMouseDown(int x, int y);
    virtual bool onMouseUp(int x, int y);
    bool isInside(int x, int y) const;

    std::shared_ptr<SVE::OverlayEntity>& getOverlay();

    static std::string getDefaultOverlayFolder();

protected:
    std::string createMaterial(const std::string& textureFile);

protected:
    static uint32_t _globalIndex;
    uint32_t _index;
    ControlType _controlType;
    std::string _name;

    int32_t _x;
    int32_t _y;
    int32_t _width;
    int32_t _height;

    std::string _text;
    glm::ivec2 _textShift = {0, 0};

    bool _visible = true;
    bool _enabled = true;
    bool _pressed = false;
    bool _mouseTransparent = false;

    std::vector<std::shared_ptr<Control>> _children;
    Control* _parent;

    std::shared_ptr<SVE::OverlayEntity> _overlay;

    std::string _defaultMaterial;
    std::string _pushMaterial;
    std::string _hoverMaterial;
    std::string _disabledMaterial;

    std::vector<IEventHandler*> _mouseDownHandlerList;
    std::vector<IEventHandler*> _mouseUpHandlerList;
    std::vector<IEventHandler*> _mouseMoveHandlerList;
};

} // namespace Chewman