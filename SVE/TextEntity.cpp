// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "TextEntity.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "MaterialManager.h"
#include "Utils.h"

namespace SVE
{

TextEntity::TextEntity(TextInfo textInfo, const std::string& fontMaterial)
    : _textInfo(std::move(textInfo))
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(fontMaterial))
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
    _renderLast = true;
}

TextInfo& TextEntity::getText()
{
    return _textInfo;
}

void TextEntity::setText(TextInfo textInfo)
{
    _textInfo = std::move(textInfo);
}

void TextEntity::updateUniforms(UniformDataList uniformDataList) const
{
    auto& uniformData = *uniformDataList[toInt(CommandsType::MainPass)];
    uniformData.textInfo.symbolCount = _textInfo.symbolCount;
    uniformData.textInfo.fontImageSize = glm::vec2(_textInfo.font->width, _textInfo.font->height);
    uniformData.textInfo.imageSize = glm::vec2(1024, 768);
    uniformData.textInfo.maxHeight = _textInfo.font->maxHeight;
    uniformData.glyphList.reserve(300);
    std::copy(_textInfo.font->symbols, _textInfo.font->symbols + 300, std::back_inserter(uniformData.glyphList));
    uniformData.textSymbolList.reserve(_textInfo.symbolCount);
    uniformData.textSymbolList = _textInfo.symbols;

    _material->getVulkanMaterial()->setUniformData(_materialIndex, *uniformDataList[toInt(CommandsType::MainPass)]);
}

void TextEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    if (Engine::getInstance()->getPassType() == CommandsType::MainPass || Engine::getInstance()->getPassType() == CommandsType::ScreenQuadPass
        || Engine::getInstance()->getPassType() == CommandsType::ScreenQuadLatePass)
    {
        auto commandBuffer = Engine::getInstance()->getVulkanInstance()->getCommandBuffer(bufferIndex);

        //VkDeviceSize offset = 0;
        //vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffer, &offset);

        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    }
}

} // namespace SVE