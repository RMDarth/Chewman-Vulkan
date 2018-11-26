// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Entity.h"
#include "ParticleSystemSettings.h"

namespace SVE
{
class VulkanComputeEntity;
class VulkanParticleSystem;
class Material;

class ParticleSystemEntity : public Entity
{
public:
    explicit ParticleSystemEntity(ParticleSystemSettings settings);
    ~ParticleSystemEntity() override;

    void applyComputeCommands() const;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;
    void updateUniforms(UniformDataList uniformDataList) const override;

    // TODO: Move to manager
    void fillUniformData(UniformData& data);

    const ParticleSystemSettings& getSettings() const;

private:
    void generateParticles();

private:
    ParticleSystemSettings _settings;
    std::unique_ptr<VulkanComputeEntity> _vulkanComputeEntity;
    std::unique_ptr<VulkanParticleSystem> _vulkanParticleSystem;

    Material* _material;
    uint32_t _materialIndex;
};

} // namespace SVE