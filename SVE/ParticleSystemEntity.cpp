// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ParticleSystemEntity.h"
#include "ParticleSystemManager.h"
#include "VulkanParticleSystem.h"
#include "VulkanComputeEntity.h"
#include "MaterialManager.h"
#include "VulkanMaterial.h"
#include "Engine.h"
#include "Utils.h"

namespace SVE
{

ParticleSystemEntity::ParticleSystemEntity(ParticleSystemSettings settings)
    : _settings(std::move(settings))
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(_settings.materialName))
{
    _renderLast = true;
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
    generateParticles();
}

ParticleSystemEntity::ParticleSystemEntity(const std::string &name)
    : ParticleSystemEntity(*Engine::getInstance()->getParticleSystemManager()->getParticleSystem(name))
{

}

ParticleSystemEntity::~ParticleSystemEntity() = default;

void ParticleSystemEntity::applyComputeCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    // TODO: Change it to VulkanCommandsManager interface
    _vulkanComputeEntity->reallocateCommandBuffers();
    //_material->getVulkanMaterial()->applyComputeCommands(bufferIndex, imageIndex, _materialIndex);
    _vulkanComputeEntity->applyComputeCommands();
}

void ParticleSystemEntity::updateUniforms(UniformDataList uniformDataList) const
{
    auto& data = *uniformDataList[toInt(CommandsType::MainPass)];
    data.particleEmitter = _settings.particleEmitter;
    data.particleAffector = _settings.particleAffector;
    data.particleCount = _settings.quota;
    data.spritesheetSize = _material->getVulkanMaterial()->getSpritesheetSize();

    _material->getVulkanMaterial()->setUniformData(_materialIndex, data);
    _vulkanComputeEntity->setUniformData(data);
}

void ParticleSystemEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    if (Engine::getInstance()->getPassType() == CommandsType::MainPass)
    {
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
        _vulkanParticleSystem->applyDrawingCommands(bufferIndex);
    }
}

ParticleSystemSettings& ParticleSystemEntity::getSettings()
{
    return _settings;
}

void ParticleSystemEntity::generateParticles()
{
    std::vector<float> particles(_settings.quota * 5 * 4, -0.00001f); // 5*vec4

    ComputeSettings computeSettings;
    computeSettings.data = particles.data();
    computeSettings.dataSize = particles.size() * sizeof(float);
    computeSettings.elementsCount = _settings.quota;
    computeSettings.computeShaderName = _settings.computeShaderName;
    computeSettings.name = _settings.name;
    _vulkanComputeEntity = std::make_unique<VulkanComputeEntity>(std::move(computeSettings));
    _vulkanParticleSystem = std::make_unique<VulkanParticleSystem>(_settings, *_vulkanComputeEntity);
}

} // namespace SVE