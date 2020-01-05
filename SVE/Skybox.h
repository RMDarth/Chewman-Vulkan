// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
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
    void updateUniforms(UniformDataList uniformDataList) const override;

private:
    void setupMaterial();

private:
    std::shared_ptr<Mesh> _mesh;
    Material* _material = nullptr;

    uint32_t _materialIndex = 0;
    uint32_t _reflectionMaterialIndex = 0;
    uint32_t _refractionMaterialIndex = 0;
};

} // namespace SVE