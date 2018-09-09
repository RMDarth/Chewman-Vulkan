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

}

MeshEntity::~MeshEntity() = default;

SubmitInfo MeshEntity::render(const UniformData& data) const
{
    _material->getVulkanMaterial()->setUniformData(data);
    return SubmitInfo(_mesh->getVulkanMesh()->createSubmitInfo());
}

void MeshEntity::setMaterial(std::string materialName)
{
    _material = Engine::getInstance()->getMaterialManager()->getMaterial(materialName);
}


} // namespace SVE