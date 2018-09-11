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

    void updateUniforms(const UniformData& data) const override;
    void applyDrawingCommands(uint32_t bufferIndex) const override;

private:
    std::shared_ptr<Mesh> _mesh;
    std::shared_ptr<Material> _material;
    uint32_t _materialIndex;
};

} // namespace SVE