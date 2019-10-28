// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

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
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(mesh->getDefaultMaterialName(), true))
    , _materialInfo { glm::vec4(0), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 16 }
{
    if (_material)
    {
        setupMaterial();
    }
}

MeshEntity::~MeshEntity()
{
    if (_material)
    {
        _material->getVulkanMaterial()->deleteInstancesForEntity(this);
    }
    if (_shadowMaterial)
    {
        _shadowMaterial->getVulkanMaterial()->deleteInstancesForEntity(this);
    }
    if (_pointLightShadowMaterial)
    {
        _pointLightShadowMaterial->getVulkanMaterial()->deleteInstancesForEntity(this);
    }
}

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

void MeshEntity::updateUniforms(UniformDataList uniformDataList) const
{
    for (auto& uniformData : uniformDataList)
    {
        // TODO: Load material info data from resources
        uniformData->materialInfo = _materialInfo;
    }

    UniformData newData = *uniformDataList[toInt(CommandsType::MainPass)];

    if (!_isTimePaused)
        _time += Engine::getInstance()->getDeltaTime();
    newData.time = _time;

    if (_animationState == AnimationState::Play && !_isTimePaused)
        _animationTime += Engine::getInstance()->getDeltaTime();
    _mesh->updateUniformDataBones(newData, _animationTime);
    _material->getVulkanMaterial()->setUniformData(_materialIndex, newData);

    if (_shadowMaterial)
    {
        UniformData newShadowData = *uniformDataList[toInt(CommandsType::ShadowPassDirectLight)];
        newShadowData.bones = newData.bones;
        _shadowMaterial->getVulkanMaterial()->setUniformData(
                _shadowIndex,
                newShadowData);

        if (_renderToDepth)
        {
            UniformData depthData = *uniformDataList[toInt(CommandsType::ScreenQuadDepthPass)];
            depthData.bones = newData.bones;
            _shadowMaterial->getVulkanMaterial()->setUniformData(
                    _depthIndex,
                    depthData);
        }
    }
    if (_pointLightShadowMaterial)
    {
        UniformData newShadowData = *uniformDataList[toInt(CommandsType::ShadowPassPointLights)];
        newShadowData.bones = newData.bones;
        _pointLightShadowMaterial->getVulkanMaterial()->setUniformData(
                _pointLightShadowMaterial->getVulkanMaterial()->getInstanceForEntity(this),
                newShadowData);
    }
    if (Engine::getInstance()->isWaterEnabled())
    {
        UniformData newReflectionData = *uniformDataList[toInt(CommandsType::ReflectionPass)];
        newReflectionData.bones = newData.bones;
        _material->getVulkanMaterial()->setUniformData(_reflectionMaterialIndex, newReflectionData);

        UniformData newRefractionData = *uniformDataList[toInt(CommandsType::RefractionPass)];
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
    else if (Engine::getInstance()->getPassType() == CommandsType::ShadowPassDirectLight)
    {
        if (!_castShadows || !_shadowMaterial)
            return;

        _shadowMaterial->getVulkanMaterial()->applyDrawingCommands(
                bufferIndex,
                imageIndex,
                _shadowIndex);
    }
    else if (Engine::getInstance()->getPassType() == CommandsType::ShadowPassPointLights)
    {
        if (!_castShadows || !_pointLightShadowMaterial)
            return;
        _pointLightShadowMaterial->getVulkanMaterial()->applyDrawingCommands(
                bufferIndex,
                imageIndex,
                _pointLightShadowMaterial->getVulkanMaterial()->getInstanceForEntity(this));
    }
    else if (Engine::getInstance()->getPassType() == CommandsType::ScreenQuadDepthPass)
    {
        if (!_renderToDepth)
            return;

        if (_shadowMaterial)
            _shadowMaterial->getVulkanMaterial()->applyDrawingCommands(
                    bufferIndex,
                    imageIndex,
                    _depthIndex);
    }
    else
    {
        bool isMRTPass = Engine::getInstance()->getPassType() == CommandsType::ScreenQuadMRTPass;
        if (isMRTPass != _material->isMRT())
            return;
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
    }

    if (!isInstanceRendering() ||
        !_material->getVulkanMaterial()->isInstancesRendered() || _material->getVulkanMaterial()->isMainInstance(_materialIndex))
    {
        _mesh->getVulkanMesh()->applyDrawingCommands(bufferIndex, _material->getVulkanMaterial()->getInstanceCount());
        _material->getVulkanMaterial()->setInstancedRendered();
        _material->getVulkanMaterial()->setMainInstance(_materialIndex);
        if (_shadowMaterial)
            _shadowMaterial->getVulkanMaterial()->setMainInstance(_materialIndex);
    }
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
        {
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("SimpleSkeletalDepth");
            //_pointLightShadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("FullSkeletalDepth");
        }
        else
        {
            _shadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial(
                    _material->getVulkanMaterial()->getSettings().useInstancing ? "SimpleDepthInstanced" : "SimpleDepth");
            //_pointLightShadowMaterial = Engine::getInstance()->getMaterialManager()->getMaterial("FullDepth");
        }

        _shadowIndex = _shadowMaterial->getVulkanMaterial()->getInstanceForEntity(this, 0);
        _depthIndex = _shadowMaterial->getVulkanMaterial()->getInstanceForEntity(this, 1);
    }
}

void MeshEntity::setMaterialInfo(const MaterialInfo& materialInfo)
{
    _materialInfo = materialInfo;
}

MaterialInfo* MeshEntity::getMaterialInfo()
{
    return &_materialInfo;
}

bool MeshEntity::isInstanceRendering() const
{
    return _material->getVulkanMaterial()->getSettings().useInstancing;
}

void MeshEntity::updateInstanceBuffers()
{
    _material->getVulkanMaterial()->updateInstancedData();
    if (_shadowMaterial)
    {
        _shadowMaterial->getVulkanMaterial()->updateInstancedData();
    }
}

void MeshEntity::setAnimationState(AnimationState animationState)
{
    _animationState = animationState;
}

void MeshEntity::resetTime(float time, bool resetAnimation)
{
    _time = time;
    if (resetAnimation)
        _animationTime = time;
}

void MeshEntity::subscribeToAttachment(const std::string& name)
{
    _mesh->subscribeToAttachment(name);
}

glm::mat4 MeshEntity::getAttachment(const std::string& name)
{
    return _mesh->getAttachment(name);
}

void MeshEntity::unsubscribeFromAttachment(const std::string& name)
{
    _mesh->unsubscribeFromAttachment(name);
}

} // namespace SVE