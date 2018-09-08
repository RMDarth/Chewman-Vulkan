// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>
#include <list>
#include "Entity.h"

namespace SVE
{

class SceneNode
{
public:
    explicit SceneNode(std::string name);
    SceneNode();

    const std::string& getName() const;

    void attachEntity(std::shared_ptr<Entity> entity);
    void detachEntity(std::shared_ptr<Entity> entity);
    const std::list<std::shared_ptr<Entity>>& getAttachedEntities() const;

    void attachSceneNode(std::shared_ptr<SceneNode> sceneNode);
    void detachSceneNode(std::shared_ptr<SceneNode> sceneNode);
    const std::list<std::shared_ptr<SceneNode>>& getChildren() const;
private:
    std::string _name;
    std::list<std::shared_ptr<Entity>> _entityList;
    std::list<std::shared_ptr<SceneNode>> _sceneNodeList;
};

} // namespace SVE