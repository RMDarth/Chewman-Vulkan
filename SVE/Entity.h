// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <memory>

namespace SVE
{
struct UniformData;
class SceneNode;

// Base class for entities that can be attached to scene nodes
class Entity : public std::enable_shared_from_this<Entity>
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    void setParent(std::shared_ptr<SceneNode> parent);
    std::shared_ptr<SceneNode> getParent();
    void detachFromParent();
    void clearParent();

    virtual void setMaterial(const std::string& materialName);

    virtual void updateUniforms(const UniformData& data) const = 0;
    virtual void applyDrawingCommands(uint32_t bufferIndex) const = 0;

protected:
    std::shared_ptr<SceneNode> _parent;
};

} // namespace SVE