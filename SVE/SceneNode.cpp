// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "SceneNode.h"
#include "SceneManager.h"
#include "Engine.h"
#include <algorithm>

namespace SVE
{

SceneNode::SceneNode(std::string name)
    : _name(std::move(name))
{

}

SceneNode::SceneNode()
{

}

const std::string &SceneNode::getName() const
{
    return _name;
}

void SceneNode::setParent(std::shared_ptr<SceneNode> parent)
{
    if (!_parent.expired())
    {
        auto currentParent = _parent.lock();
        if (currentParent)
        {
            currentParent->detachSceneNode(shared_from_this());
        }
    }
    _parent = std::move(parent);
}

std::shared_ptr<SceneNode> SceneNode::getParent() const
{
    return _parent.lock();
}

void SceneNode::attachEntity(std::shared_ptr<Entity> entity)
{
    entity->setParent(shared_from_this());
    _entityList.push_back(std::move(entity));
}

void SceneNode::detachEntity(std::shared_ptr<Entity> entity)
{
    if (entity->getParent().get() == this)
    {
        _entityList.remove(entity);
        entity->clearParent();
    }
}

const std::list<std::shared_ptr<Entity>>& SceneNode::getAttachedEntities() const
{
    if (_entitiesHidden)
    {
        static std::list<std::shared_ptr<Entity>> emptyList {};
        return emptyList;
    }

    return _entityList;
}

void SceneNode::setHideEntities(bool value)
{
    _entitiesHidden = value;
}


void SceneNode::attachSceneNode(std::shared_ptr<SceneNode> sceneNode)
{
    sceneNode->setParent(shared_from_this());
    _sceneNodeList.push_back(sceneNode);
}

void SceneNode::detachSceneNode(std::shared_ptr<SceneNode> sceneNode)
{
    if (sceneNode->getParent().get() == this)
    {
        _sceneNodeList.remove(sceneNode);
        sceneNode->_parent.reset();
    }
}

const std::list<std::shared_ptr<SceneNode>>& SceneNode::getChildren() const
{
    return _sceneNodeList;
}

glm::mat4 SceneNode::getNodeTransformation() const
{
    if (!_attachment)
        return _transformation;
    else
        return _attachment->getAttachment(_attachmentName) * _transformation;
}

void SceneNode::setNodeTransformation(glm::mat4 transform)
{
    _transformation = std::move(transform);
}

glm::mat4 SceneNode::getTotalTransformation() const
{
    glm::mat4 totalTransform = glm::mat4(1);
    auto node = shared_from_this();
    while (node)
    {
        totalTransform *= node->getNodeTransformation();
        node = node->getParent();
    }

    return totalTransform;
}

void SceneNode::setCurrentFrame(uint64_t frame)
{
    _currentFrame = frame;
}

uint64_t SceneNode::getCurrentFrame() const
{
    return _currentFrame;
}

void SceneNode::setEntityAttachment(std::shared_ptr<Entity> entity, const std::string& attachmentName)
{
    if (_attachment)
    {
        _attachment->unsubscribeFromAttachment(_attachmentName);
    }

    _attachment = std::move(entity);
    _attachment->subscribeToAttachment(attachmentName);
    _attachmentName = attachmentName;
}

} // namespace SVE