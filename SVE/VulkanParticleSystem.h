// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "VulkanHeaders.h"
#include "ParticleSystemSettings.h"
#include <memory>
#include <vector>

namespace SVE
{

class VulkanUtils;
class VulkanInstance;
class VulkanComputeEntity;

class VulkanParticleSystem
{
public:
    explicit VulkanParticleSystem(const ParticleSystemSettings& particleSettings,
                                  const VulkanComputeEntity& particleComputeEntity);
    ~VulkanParticleSystem();

    void applyDrawingCommands(uint32_t bufferIndex);


private:
    VulkanInstance* _vulkanInstance;
    const VulkanUtils& _vulkanUtils;

    const ParticleSystemSettings& _particleSettings;
    VkBuffer _buffer;
};

} // namespace SVE