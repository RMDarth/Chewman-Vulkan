// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "SceneNode.h"
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

void SceneNode::attachEntity(std::shared_ptr<Entity> entity)
{
    _entityList.push_back(std::move(entity));
}

void SceneNode::detachEntity(std::shared_ptr<Entity> entity)
{
    _entityList.remove(entity);
}

const std::list<std::shared_ptr<Entity>> &SceneNode::getAttachedEntities() const
{
    return _entityList;
}

void SceneNode::attachSceneNode(std::shared_ptr<SceneNode> sceneNode)
{
    _sceneNodeList.push_back(sceneNode);
}

void SceneNode::detachSceneNode(std::shared_ptr<SceneNode> sceneNode)
{
    _sceneNodeList.remove(sceneNode);
}

const std::list<std::shared_ptr<SceneNode>> &SceneNode::getChildren() const
{
    return _sceneNodeList;
}


} // namespace SVE