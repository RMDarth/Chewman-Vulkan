// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "ParticleSystemSettings.h"
#include <unordered_map>

namespace SVE
{

class ParticleSystemManager
{
public:
    ParticleSystemManager();
    ~ParticleSystemManager();

    void registerParticleSystem(ParticleSystemSettings particleSystem);
    ParticleSystemSettings* getParticleSystem(const std::string& name);
    const ParticleSystemSettings* getParticleSystem(const std::string& name) const;

private:
    std::unordered_map<std::string, ParticleSystemSettings> _particleSystemMap;
};

} // namespace SVE