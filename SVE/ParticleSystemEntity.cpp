// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ParticleSystemEntity.h"
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
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
    generateParticles();
}

ParticleSystemEntity::~ParticleSystemEntity() = default;

void ParticleSystemEntity::applyComputeCommands() const
{
    _vulkanComputeEntity->applyComputeCommands();
}

void ParticleSystemEntity::updateUniforms(UniformDataList uniformDataList) const
{
    _material->getVulkanMaterial()->setUniformData(_materialIndex, *uniformDataList[toInt(CommandsType::MainPass)]);
    _vulkanComputeEntity->setUniformData(*uniformDataList[toInt(CommandsType::MainPass)]);
}

void ParticleSystemEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    if (Engine::getInstance()->getPassType() == CommandsType::MainPass)
    {
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
        _vulkanParticleSystem->applyDrawingCommands(bufferIndex);
    }
}

void ParticleSystemEntity::generateParticles()
{
    std::vector<float> particles;

    for (uint32_t i = 0; i < _settings.quota; i++)
    {
        // generate particles
        float radius = 1.5f;
        float angle = (360.0f / _settings.quota) * i;
        glm::vec3 position = glm::vec3 ( radius * cos(glm::radians(angle)), 0, radius * sin(glm::radians(angle)));
        glm::vec3 color = 0.0075f * glm::vec3 {
                static_cast<float>(std::rand() % 61 + 20),
                static_cast<float>(std::rand() % 61 + 40),
                static_cast<float>(std::rand() % 61)
        };
        float speed = 0.5f + 0.01f * static_cast<float>(std::rand() % 101) + color[0] * 0.5f;

        particles.push_back(position.x);
        particles.push_back(position.y);
        particles.push_back(position.z);
        particles.push_back(1.0f);

        particles.push_back(color.x);
        particles.push_back(color.y);
        particles.push_back(color.z);
        particles.push_back(speed);
    }

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