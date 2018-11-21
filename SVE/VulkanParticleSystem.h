// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "ParticleSystemSettings.h"

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