// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "MeshEntity.h"
#include "Engine.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "VulkanMesh.h"
#include "VulkanMaterial.h"

namespace SVE
{

MeshEntity::MeshEntity(std::string name)
    : MeshEntity(Engine::getInstance()->getMeshManager()->getMesh(name))
{

}

MeshEntity::MeshEntity(std::shared_ptr<Mesh> mesh)
    : _mesh(mesh)
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(mesh->getDefaultMaterialName()))
{
    if (_material)
    {
        setupMaterial();
    }
}

MeshEntity::~MeshEntity() = default;

void MeshEntity::setMaterial(const std::string& materialName)
{
    _material = Engine::getInstance()->getMaterialManager()->getMaterial(materialName);
    setupMaterial();
}

void MeshEntity::updateUniforms(const UniformData& data, bool shadow) const
{
    UniformData newData = data;
    _mesh->updateUniformDataBones(newData, Engine::getInstance()->getTime());
    if (!shadow)
    {
        _material->getVulkanMaterial()->setUniformData(_materialIndex, newData);
    } else
    {
        if (_shadowMaterial)
            _shadowMaterial->getVulkanMaterial()->setUniformData(_shadowMaterialIndex, newData);
    }
}

void MeshEntity::applyDrawingCommands(uint32_t bufferIndex, bool applyMaterial) const
{
    if (!applyMaterial)
    {
        if (_shadowMaterial)
            _shadowMaterial->getVulkanMaterial()->applyDrawingCommands(bufferIndex, _shadowMaterialIndex);
    } else {
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, _materialIndex);
    }

    _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex);
}

void MeshEntity::setupMaterial()
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);

    if (Engine::getInstance()->isShadowMappingEnabled())
    {
        if (_material->getVulkanMaterial()->isSkeletal())
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("SimpleSkeletalDepth");
        else
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("SimpleDepth");

        _shadowMaterialIndex = _shadowMaterial->getVulkanMaterial()->getInstanceForEntity(this);
    }
}

} // namespace SVE