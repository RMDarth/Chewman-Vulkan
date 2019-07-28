// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "ParticleSystemManager.h"

namespace SVE
{

ParticleSystemManager::ParticleSystemManager()
{

}

ParticleSystemManager::~ParticleSystemManager()
{

}

void ParticleSystemManager::addParticleSystem(std::shared_ptr<ParticleSystemEntity> particleSystem)
{
    _particleSystemList.push_back(particleSystem);
}

void ParticleSystemManager::removeParticleSystem(uint32_t index)
{
    if (index < _particleSystemList.size())
    {
        _particleSystemList.erase(_particleSystemList.begin() + index);
    }
}

std::shared_ptr<ParticleSystemEntity> ParticleSystemManager::getParticleSystem(uint32_t index) const
{
    return index < _particleSystemList.size() ? _particleSystemList[index] : std::shared_ptr<ParticleSystemEntity>();
}

size_t ParticleSystemManager::getParticleSystemCount() const
{
    return _particleSystemList.size();
}

void ParticleSystemManager::fillUniformData(UniformData &data)
{
    for (auto i = 0u; i < _particleSystemList.size(); i++)
    {
        if (_particleSystemList[i])
            _particleSystemList[i]->fillUniformData(data);
    }
}

void ParticleSystemManager::applyComputeCommands(uint32_t bufferIndex, uint32_t imageIndex)
{
    ParticleSystemEntity::startComputeStep();
    for (auto i = 0u; i < _particleSystemList.size(); i++)
    {
        if (_particleSystemList[i])
            _particleSystemList[i]->applyComputeCommands(bufferIndex, imageIndex);
    }

    ParticleSystemEntity::finishComputeStep();
}

} // namespace SVE