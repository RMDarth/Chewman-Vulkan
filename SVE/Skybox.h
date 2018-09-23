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

    void applyDrawingCommands(uint32_t bufferIndex, bool applyMaterial) const override;
    void updateUniforms(const UniformData& data, bool shadow) const override;

private:
    std::shared_ptr<Mesh> _mesh;
    std::shared_ptr<Material> _material;
    uint32_t _materialIndex;
};

} // namespace SVE