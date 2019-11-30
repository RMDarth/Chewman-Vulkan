// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <SVE/Entity.h>
#include <SVE/Engine.h>
#include <SVE/MaterialManager.h>
#include <SVE/Utils.h>
#include <SVE/VulkanMaterial.h>
#include <SVE/VulkanInstance.h>

namespace SVE
{
class Material;
}

namespace Chewman
{

template<typename InfoStruct>
class CustomEntity : public SVE::Entity
{
public:
    CustomEntity(const std::string& material, InfoStruct startInfo)
        : _material(SVE::Engine::getInstance()->getMaterialManager()->getMaterial(material))
        , _currentInfo(startInfo)
    {
        _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
        _renderLast = true;
    }

    void setMaterial(const std::string& materialName) override
    {
        _material = SVE::Engine::getInstance()->getMaterialManager()->getMaterial(materialName);
        _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
    }

    void updateInfo(InfoStruct info)
    {
        _currentInfo = info;
    }

    InfoStruct& getInfo()
    {
        return _currentInfo;
    }

    void updateUniforms(SVE::UniformDataList uniformDataList) const override
    {
        auto mainUniform = uniformDataList[toInt(SVE::CommandsType::MainPass)];
        mainUniform->customMat4 = _currentInfo.toMat4();
        mainUniform->spritesheetSize = _material->getVulkanMaterial()->getSpritesheetSize();
        mainUniform->materialInfo.diffuse = glm::vec4(1);

        _material->getVulkanMaterial()->setUniformData(_materialIndex, *mainUniform);
    }

    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override
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

private:
    SVE::Material* _material = nullptr;
    uint32_t _materialIndex = 0;
    InfoStruct _currentInfo;
};

} // namespace Chewman