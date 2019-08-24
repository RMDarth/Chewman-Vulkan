// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "PostEffectManager.h"
#include "Material.h"
#include "VulkanPostEffect.h"
#include "MaterialManager.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "Utils.h"

namespace SVE
{

PostEffectManager::~PostEffectManager() = default;
PostEffect::~PostEffect() = default;

void PostEffectManager::addPostEffect(const std::string& materialName, std::string effectName, int width, int height)
{
    auto* engine = Engine::getInstance();

    PostEffect postEffect {};
    postEffect.index = _effectList.size() + 1;
    postEffect.name = std::move(effectName);
    postEffect.width = width < 0 ? engine->getRenderWindowSize().x : width;
    postEffect.height = height < 0 ? engine->getRenderWindowSize().y : height;
    postEffect.material = Engine::getInstance()->getMaterialManager()->getMaterial(materialName);
    postEffect.material->getVulkanMaterial()->updateDescriptorSets();
    postEffect.materialIndex = postEffect.material->getVulkanMaterial()->getInstanceForEntity(nullptr);
    postEffect.vulkanPostEffect = std::make_unique<VulkanPostEffect>(postEffect.index, width, height);

    _effectMap[postEffect.name] = postEffect.index;
    _effectList.push_back(std::move(postEffect));

    engine->getMaterialManager()->getMaterial("ScreenQuad")->getVulkanMaterial()->updateDescriptorSets();
}

uint32_t PostEffectManager::getEffectIndex(const std::string& name)
{
    auto iter = _effectMap.find(name);
    return iter == _effectMap.end() ? 0 : iter->second;
}

void PostEffectManager::createCommands(uint32_t currentFrame, uint32_t currentImage)
{
    for (auto& postEffect : _effectList)
    {
        auto bufferIndex = BUFFER_INDEX_SCREEN_QUAD + postEffect.index;
        auto commandBuffer =  postEffect.vulkanPostEffect->reallocateCommandBuffers();

        postEffect.vulkanPostEffect->startRenderCommandBufferCreation();
        postEffect.material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, currentImage, postEffect.materialIndex);

        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        postEffect.vulkanPostEffect->endRenderCommandBufferCreation();
    }
}

void PostEffectManager::submitCommands(UniformDataList uniformDataList)
{
    auto* vulkanInstance = Engine::getInstance()->getVulkanInstance();
    auto* uniformData = uniformDataList[toInt(CommandsType::ScreenQuadPass)].get();
    for (auto& postEffect : _effectList)
    {
        postEffect.material->getVulkanMaterial()->setUniformData(postEffect.materialIndex, *uniformData);
        vulkanInstance->submitCommands(CommandsType::ScreenQuadPass, BUFFER_INDEX_SCREEN_QUAD + postEffect.index);
        uniformData->imageSize = glm::ivec4(postEffect.width, postEffect.height, 0, 0);
    }
}

} // namespace SVE