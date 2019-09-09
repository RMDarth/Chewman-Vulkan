// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include <utility>
#include "Entity.h"
#include "SceneNode.h"

namespace SVE
{

void Entity::setParent(std::shared_ptr<SceneNode> parent)
{
    if (_parent)
    {
        _parent->detachEntity(shared_from_this());
    }
    _parent = std::move(parent);
}

std::shared_ptr<SceneNode> Entity::getParent()
{
    return _parent;
}

void Entity::detachFromParent()
{
    if (_parent)
    {
        _parent->detachEntity(shared_from_this());
    }
    _parent.reset();
}

void Entity::clearParent()
{
    _parent.reset();
}

void Entity::setMaterial(const std::string& materialName)
{
    // default do nothing
}

void Entity::setRenderLast()
{
    _renderLast = true;
}

bool Entity::isRenderLast() const
{
    return _renderLast;
}

bool Entity::isInstanceRendering() const
{
    return false;
}

bool Entity::isComputeEntity() const
{
    return false;
}

void Entity::setMaterialInfo(const MaterialInfo& materialInfo)
{
    // do nothing by default
}

MaterialInfo* Entity::getMaterialInfo()
{
    return nullptr;
}

void Entity::updateInstanceBuffers()
{
    // do nothing
}

bool Entity::isRenderToDepth() const
{
    return _renderToDepth;
}

void Entity::setRenderToDepth(bool renderToDepth)
{
    _renderToDepth = renderToDepth;
}

} // namespace SVE