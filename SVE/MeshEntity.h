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
    explicit MeshEntity(Mesh* mesh);
    ~MeshEntity();

    void setMaterial(const std::string& materialName) override;
    void setCastShadows(bool castShadows);

    // TODO: add IsRefracted method
    void setIsReflected(bool isReflected);

    void updateUniforms(UniformDataList uniformDataList) const override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;

private:
    void setupMaterial();

private:
    Mesh* _mesh;
    Material* _material;
    bool _isReflected = true;
    bool _castShadows = true;

    uint32_t _materialIndex;
    uint32_t _reflectionMaterialIndex;
    uint32_t _refractionMaterialIndex;

    Material* _shadowMaterial;
    Material* _pointLightShadowMaterial;
    //std::vector<uint32_t> _shadowMaterialIndexes;
};

} // namespace SVE