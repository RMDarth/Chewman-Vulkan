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
    _materialIndex = _material->getVulkanMaterial()->getNewInstance();
}

MeshEntity::~MeshEntity() = default;

void MeshEntity::setMaterial(const std::string& materialName)
{
    _material = Engine::getInstance()->getMaterialManager()->getMaterial(materialName);
    _materialIndex = _material->getVulkanMaterial()->getNewInstance();
}

void MeshEntity::updateUniforms(const UniformData& data) const
{
    _material->getVulkanMaterial()->setUniformData(_materialIndex, data);
}

void MeshEntity::applyDrawingCommands(uint32_t bufferIndex) const
{
    _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex, _material->getVulkanMaterial(), _materialIndex);
}

} // namespace SVE