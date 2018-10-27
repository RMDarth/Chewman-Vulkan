// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Entity.h"
#include <string>
#include <vector>
#include <memory>

namespace SVE
{
class Mesh;
class Material;

class Skybox : public Entity
{
public:
    explicit Skybox(const std::string& materialName);
    ~Skybox() override;

    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;
    void updateUniforms(UniformDataList uniformDataList, const UniformDataIndexMap& indexMap) const override;

private:
    void setupMaterial();

private:
    std::shared_ptr<Mesh> _mesh;
    Material* _material;

    uint32_t _materialIndex;
    uint32_t _reflectionMaterialIndex;
    uint32_t _refractionMaterialIndex;
};

} // namespace SVE