// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "ParticleSystemManager.h"

namespace SVE
{

ParticleSystemManager::ParticleSystemManager()
{

}

ParticleSystemManager::~ParticleSystemManager()
{

}

void ParticleSystemManager::registerParticleSystem(ParticleSystemSettings particleSystem)
{
    _particleSystemMap.emplace(particleSystem.name, particleSystem);
}

const ParticleSystemSettings* ParticleSystemManager::getParticleSystem(const std::string &name) const
{
    auto ps = _particleSystemMap.find(name);
    if (ps != _particleSystemMap.end())
    {
        return &ps->second;
    }
    return nullptr;
}

ParticleSystemSettings* ParticleSystemManager::getParticleSystem(const std::string &name)
{
    auto ps = _particleSystemMap.find(name);
    if (ps != _particleSystemMap.end())
    {
        return &ps->second;
    }
    return nullptr;
}

} // namespace SVE