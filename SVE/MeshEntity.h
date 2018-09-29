// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Entity.h"
#include <memory>

namespace SVE
{
class Mesh;
class Material;

class MeshEntity : public Entity
{
public:
    explicit MeshEntity(std::string name);
    explicit MeshEntity(std::shared_ptr<Mesh> mesh);
    ~MeshEntity();

    void setMaterial(const std::string& materialName) override;

    void updateUniforms(UniformDataList uniformDataList) const override;
    void applyDrawingCommands(uint32_t bufferIndex, bool applyMaterial) const override;

private:
    void setupMaterial();

private:
    std::shared_ptr<Mesh> _mesh;
    std::shared_ptr<Material> _material;
    bool _waterMaterial = false;

    uint32_t _materialIndex;
    uint32_t _reflectionMaterialIndex;

    std::shared_ptr<Material> _shadowMaterial;
    uint32_t _shadowMaterialIndex;
};

} // namespace SVE