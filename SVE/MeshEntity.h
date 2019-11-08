// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "Entity.h"
#include "ShaderSettings.h"
#include "MeshDefs.h"
#include <memory>

namespace SVE
{
class Mesh;
class Material;

enum class AnimationState : uint8_t
{
    Play,
    Pause
};

class MeshEntity : public Entity
{
public:
    explicit MeshEntity(std::string name);
    explicit MeshEntity(Mesh* mesh);
    ~MeshEntity();

    void setMaterial(const std::string& materialName) override;
    void setMaterialInfo(const MaterialInfo& materialInfo) override;
    MaterialInfo* getMaterialInfo() override;
    void setCastShadows(bool castShadows);

    // TODO: add IsRefracted method
    void setIsReflected(bool isReflected);

    void updateUniforms(UniformDataList uniformDataList) const override;
    void updateInstanceBuffers() override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;

    bool isInstanceRendering() const override;

    void setAnimationState(AnimationState animationState);
    void resetTime(float time = 0.0f, bool resetAnimation = false);

    void subscribeToAttachment(const std::string& name) override;
    void unsubscribeFromAttachment(const std::string& name) override;
    glm::mat4 getAttachment(const std::string& name) override;

private:
    void setupMaterial();

private:
    Mesh* _mesh = nullptr;
    Material* _material = nullptr;
    MaterialInfo _materialInfo;
    bool _isReflected = true;
    bool _castShadows = true;

    uint32_t _materialIndex;
    uint32_t _reflectionMaterialIndex;
    uint32_t _refractionMaterialIndex;

    Material* _shadowMaterial = nullptr;
    uint32_t _shadowIndex;
    uint32_t _depthIndex;
    Material* _pointLightShadowMaterial = nullptr;
    std::unique_ptr<Material> _bloomMaterial;
    //std::vector<uint32_t> _shadowMaterialIndexes;

    // TODO: Move to animation class
    AnimationState _animationState = AnimationState::Play;
    mutable float _animationTime = 0.0f;
    mutable float _time = 0.0f;

    mutable BonesAttachments _attachments;
};

} // namespace SVE