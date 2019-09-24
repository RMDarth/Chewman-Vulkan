// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include <vector>
#include <map>

namespace SVE
{
struct UniformData;
class SceneNode;
struct MaterialInfo;
enum class CommandsType : uint8_t;

using UniformDataList = std::vector<std::shared_ptr<UniformData>>;

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

    // TODO: Refactor, provide more agile ordering mechanism
    void setRenderLast(bool value = true);
    bool isRenderLast() const;

    // This is for special render to depth texture pass
    bool isRenderToDepth() const;
    void setRenderToDepth(bool renderToDepth);
    void pauseTime();
    void unpauseTime();

    virtual bool isComputeEntity() const;
    virtual bool isInstanceRendering() const;

    virtual void setMaterial(const std::string& materialName);
    virtual void setMaterialInfo(const MaterialInfo& materialInfo);
    virtual MaterialInfo* getMaterialInfo();

    virtual void updateUniforms(UniformDataList uniformDataList) const = 0;
    virtual void updateInstanceBuffers();
    virtual void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const = 0;

protected:
    bool _isTimePaused = false;
    float _pauseTime = 0;

    bool _renderLast = false;
    bool _renderToDepth = false;
    std::weak_ptr<SceneNode> _parent;
};

} // namespace SVE