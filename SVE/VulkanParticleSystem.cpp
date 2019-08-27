// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "VulkanParticleSystem.h"
#include "Engine.h"
#include "VulkanInstance.h"
#include "VulkanUtils.h"
#include "VulkanComputeEntity.h"

namespace SVE
{

VulkanParticleSystem::VulkanParticleSystem(const ParticleSystemSettings& particleSettings,
                                           const VulkanComputeEntity& particleComputeEntity)
        : _vulkanInstance(Engine::getInstance()->getVulkanInstance())
        , _vulkanUtils(_vulkanInstance->getVulkanUtils())
        , _particleSettings(particleSettings)
        , _buffer(particleComputeEntity.getComputeBuffer())
{

}

VulkanParticleSystem::~VulkanParticleSystem()
{
}

void VulkanParticleSystem::applyDrawingCommands(uint32_t bufferIndex)
{
    auto commandBuffer = _vulkanInstance->getCommandBuffer(bufferIndex);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffer, &offset);

    vkCmdDraw(commandBuffer, _particleSettings.quota, 1, 0, 0);
}


} // namespace SVE