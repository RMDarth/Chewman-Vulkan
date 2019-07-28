// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "ParticleSystemEntity.h"

namespace SVE
{

class ParticleSystemManager
{
public:
    ParticleSystemManager();
    ~ParticleSystemManager();

    void addParticleSystem(std::shared_ptr<ParticleSystemEntity> particleSystem);
    void removeParticleSystem(uint32_t index);
    std::shared_ptr<ParticleSystemEntity> getParticleSystem(uint32_t index) const;
    size_t getParticleSystemCount() const;

    void fillUniformData(UniformData& data);
    void applyComputeCommands(uint32_t bufferIndex, uint32_t imageIndex);

private:
    std::vector<std::shared_ptr<ParticleSystemEntity>> _particleSystemList;
};

} // namespace SVE