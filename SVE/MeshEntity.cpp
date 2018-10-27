// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "MeshEntity.h"
#include "Engine.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "VulkanMesh.h"
#include "VulkanMaterial.h"
#include "ShaderSettings.h"
#include "Utils.h"

namespace SVE
{

MeshEntity::MeshEntity(std::string name)
    : MeshEntity(Engine::getInstance()->getMeshManager()->getMesh(name))
{

}

MeshEntity::MeshEntity(Mesh* mesh)
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

void MeshEntity::setCastShadows(bool castShadows)
{
    _castShadows = castShadows;
}

void MeshEntity::setIsReflected(bool isReflected)
{
    _isReflected = isReflected;
}

void MeshEntity::updateUniforms(UniformDataList uniformDataList, const UniformDataIndexMap& indexMap) const
{
    for (auto& uniformData : uniformDataList)
    {
        // TODO: Load material info data from resources
        MaterialInfo materialInfo { glm::vec4(0), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 16 };
        uniformData->materialInfo = std::move(materialInfo);
    }

    UniformData newData = *uniformDataList[indexMap.at(CommandsType::MainPass)];

    _mesh->updateUniformDataBones(newData, Engine::getInstance()->getTime());
    _material->getVulkanMaterial()->setUniformData(_materialIndex, newData);

    if (_shadowMaterial)
    {
        auto nextPassType = static_cast<CommandsType>(toInt(CommandsType::ShadowPass) + 1);
        uint32_t lightIndex = 0;
        for (auto i = indexMap.at(CommandsType::ShadowPass); i < indexMap.at(nextPassType); i++)
        {
            UniformData newShadowData = *uniformDataList[i];
            newShadowData.bones = newData.bones;
            _shadowMaterial->getVulkanMaterial()->setUniformData(
                    _shadowMaterial->getVulkanMaterial()->getInstanceForEntity(this, lightIndex),
                    newShadowData);
            lightIndex++;
        }
    }
    if (Engine::getInstance()->isWaterEnabled())
    {
        UniformData newReflectionData = *uniformDataList[indexMap.at(CommandsType::ReflectionPass)];
        newReflectionData.bones = newData.bones;
        _material->getVulkanMaterial()->setUniformData(_reflectionMaterialIndex, newReflectionData);

        UniformData newRefractionData = *uniformDataList[indexMap.at(CommandsType::RefractionPass)];
        newRefractionData.bones = newData.bones;
        _material->getVulkanMaterial()->setUniformData(_refractionMaterialIndex, newRefractionData);
    }
}

void MeshEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    if (Engine::getInstance()->getPassType() == CommandsType::ReflectionPass)
    {
        if (!_isReflected)
            return;
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _reflectionMaterialIndex);

    } else if (Engine::getInstance()->getPassType() == CommandsType::RefractionPass)
    {
        if (!_isReflected)
            return;

        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _refractionMaterialIndex);
    }
    else if (Engine::getInstance()->getPassType() == CommandsType::ShadowPass)
    {
        if (_shadowMaterial && _castShadows)
            _shadowMaterial->getVulkanMaterial()->applyDrawingCommands(
                    bufferIndex,
                    imageIndex,
                    _shadowMaterial->getVulkanMaterial()->getInstanceForEntity(this, Engine::getInstance()->getPassSubIndex()));
    }
    else
    {
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
    }

    _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex);
}

void MeshEntity::setupMaterial()
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);

    if (Engine::getInstance()->isWaterEnabled())
    {
        _reflectionMaterialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this, 1);
        _refractionMaterialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this, 2);
    }

    if (Engine::getInstance()->isShadowMappingEnabled())
    {
        // TODO: Get shadow materials (or their names) from shadowmap class or special function in MatManager
        if (_material->getVulkanMaterial()->isSkeletal())
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("SimpleSkeletalDepth");
        else
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("SimpleDepth");
    }
}

} // namespace SVE