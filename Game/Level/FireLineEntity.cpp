// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "FireLineEntity.h"
#include "SVE/MaterialManager.h"
#include "SVE/VulkanMaterial.h"
#include "SVE/VulkanInstance.h"
#include <SVE/Utils.h>

namespace Chewman
{

FireLineEntity::FireLineEntity(const std::string& material, FireLineInfo startInfo)
    : _material(SVE::Engine::getInstance()->getMaterialManager()->getMaterial(material))
    , _currentInfo(startInfo)
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
}

void FireLineEntity::updateInfo(FireLineInfo info)
{
    _currentInfo = info;
}

FireLineInfo& FireLineEntity::getInfo()
{
    return _currentInfo;
}

void FireLineEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    auto passType = SVE::Engine::getInstance()->getPassType();
    if (passType == SVE::CommandsType::MainPass
        || passType == SVE::CommandsType::ScreenQuadPass
        || passType == SVE::CommandsType::ScreenQuadLatePass)
    {
        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
        auto commandBuffer = SVE::Engine::getInstance()->getVulkanInstance()->getCommandBuffer(bufferIndex);
        vkCmdDraw(commandBuffer, _currentInfo.maxParticles, 1, 0, 0);
    }
}

void FireLineEntity::updateUniforms(SVE::UniformDataList uniformDataList) const
{
    auto mainUniform = uniformDataList[toInt(SVE::CommandsType::MainPass)];
    mainUniform->customMat4 = glm::mat4(
            glm::vec4(_currentInfo.startPos, 1),
            glm::vec4(_currentInfo.direction, 1),
            glm::vec4(_currentInfo.percent, _currentInfo.maxParticles, _currentInfo.maxLength, _currentInfo.alpha),
            glm::vec4(0.0));
    mainUniform->spritesheetSize = _material->getVulkanMaterial()->getSpritesheetSize();

    _material->getVulkanMaterial()->setUniformData(_materialIndex, *mainUniform);
}

} // namespace Chewman