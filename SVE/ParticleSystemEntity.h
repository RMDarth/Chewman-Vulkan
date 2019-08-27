// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "ComputeEntity.h"
#include "ParticleSystemSettings.h"
#include "ShaderSettings.h"

namespace SVE
{
class VulkanComputeEntity;
class VulkanParticleSystem;
class Material;

class ParticleSystemEntity : public ComputeEntity
{
public:
    explicit ParticleSystemEntity(ParticleSystemSettings settings);
    explicit ParticleSystemEntity(const std::string& name);
    ~ParticleSystemEntity() override;

    void applyComputeCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;
    void updateUniforms(UniformDataList uniformDataList) const override;

    void setMaterialInfo(const MaterialInfo& materialInfo) override;
    MaterialInfo* getMaterialInfo() override;

    ParticleSystemSettings& getSettings();

private:
    void generateParticles();

private:
    ParticleSystemSettings _settings;
    std::unique_ptr<VulkanComputeEntity> _vulkanComputeEntity;
    std::unique_ptr<VulkanParticleSystem> _vulkanParticleSystem;

    Material* _material;
    uint32_t _materialIndex;
    MaterialInfo _materialInfo;
};

} // namespace SVE