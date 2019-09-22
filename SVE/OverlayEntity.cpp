// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "OverlayEntity.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "MaterialManager.h"
#include "FontManager.h"
#include "Utils.h"

namespace SVE
{

OverlayEntity::OverlayEntity(OverlayInfo overlayInfo)
    : _overlayInfo(std::move(overlayInfo))
    , _material(Engine::getInstance()->getMaterialManager()->getMaterial(_overlayInfo.materialName))
{
    _materialIndex = _material->getVulkanMaterial()->getInstanceForEntity(this);
    _renderLast = true;
    initText();
}

OverlayEntity::~OverlayEntity()
{
    _material->getVulkanMaterial()->deleteInstancesForEntity(this);
    if (_textMaterial)
        _textMaterial->getVulkanMaterial()->deleteInstancesForEntity(this);
}

OverlayInfo& OverlayEntity::getInfo()
{
    return _overlayInfo;
}

void OverlayEntity::setText(TextInfo textInfo)
{
    _overlayInfo.textInfo = std::move(textInfo);
    initText();
}

void OverlayEntity::updateUniforms(UniformDataList uniformDataList) const
{
    auto& uniformData = *uniformDataList[toInt(CommandsType::MainPass)];

    uniformData.overlayInfo.x = _overlayInfo.x;
    uniformData.overlayInfo.y = _overlayInfo.y;
    uniformData.overlayInfo.width = _overlayInfo.width;
    uniformData.overlayInfo.height = _overlayInfo.height;
    _material->getVulkanMaterial()->setUniformData(_materialIndex, uniformData);

    if (_overlayInfo.textInfo.symbolCount)
    {
        uniformData.textInfo.symbolCount = _overlayInfo.textInfo.symbolCount;
        uniformData.textInfo.fontImageSize = glm::vec2(_overlayInfo.textInfo.font->width, _overlayInfo.textInfo.font->height);
        uniformData.textInfo.imageSize = Engine::getInstance()->getRenderWindowSize();
        uniformData.textInfo.maxHeight = _overlayInfo.textInfo.font->maxHeight;
        uniformData.textInfo.scale = _overlayInfo.textInfo.scale;
        uniformData.glyphList.clear();
        uniformData.glyphList.reserve(300);
        std::copy(_overlayInfo.textInfo.font->symbols, _overlayInfo.textInfo.font->symbols + 300, std::back_inserter(uniformData.glyphList));
        uniformData.glyphList.resize(300);
        uniformData.textSymbolList.reserve(_overlayInfo.textInfo.symbolCount);
        uniformData.textSymbolList = _overlayInfo.textInfo.symbols;

        _textMaterial->getVulkanMaterial()->setUniformData(_textMaterialIndex, uniformData);
    }
}

void OverlayEntity::applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const
{
    if (Engine::getInstance()->getPassType() == CommandsType::MainPass
        || Engine::getInstance()->getPassType() == CommandsType::ScreenQuadPass
        || Engine::getInstance()->getPassType() == CommandsType::ScreenQuadLatePass)
    {
        auto commandBuffer = Engine::getInstance()->getVulkanInstance()->getCommandBuffer(bufferIndex);

        _material->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _materialIndex);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        if (_textMaterial)
        {
            _textMaterial->getVulkanMaterial()->applyDrawingCommands(bufferIndex, imageIndex, _textMaterialIndex);
            vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        }
    }
}

void OverlayEntity::initText()
{
    if (_textMaterial)
    {
        _textMaterial->getVulkanMaterial()->deleteInstancesForEntity(this);
    }
    if (_overlayInfo.textInfo.symbolCount)
    {
        auto& textInfo = _overlayInfo.textInfo;
        auto textSize = textInfo.textSize;
        int textX = 0, textY = 0;
        switch (_overlayInfo.textHAlignment)
        {
            case TextAlignment::Left:
                textX = _overlayInfo.x;
                break;
            case TextAlignment::Center:
                textX = _overlayInfo.x + _overlayInfo.width / 2 - textSize.x / 2;
                break;
            case TextAlignment::Right:
                textX = _overlayInfo.x + _overlayInfo.width - textSize.x;
                break;
        }
        auto& someSymbol = textInfo.font->symbols[textInfo.font->symbolToInfoPos['A']];
        auto shiftToSymbolH = textInfo.scale * (textInfo.font->maxHeight - someSymbol.originY);
        switch (_overlayInfo.textVAlignment)
        {
            case TextVerticalAlignment::Top:
                textY = _overlayInfo.y;
                break;
            case TextVerticalAlignment::Center:
                textY = _overlayInfo.y + _overlayInfo.height / 2 - shiftToSymbolH - textInfo.scale * someSymbol.height / 2;
                break;
            case TextVerticalAlignment::Bottom:
                textY = _overlayInfo.y + _overlayInfo.height - shiftToSymbolH - textSize.y;
                break;
        }

        _overlayInfo.textInfo = Engine::getInstance()->getFontManager()->generateText(textInfo.text, textInfo.font->fontName, textInfo.scale, glm::ivec2(textX, textY) );

        _textMaterial = Engine::getInstance()->getMaterialManager()->getMaterial(_overlayInfo.textInfo.font->materialName);
        _textMaterialIndex = _textMaterial->getVulkanMaterial()->getInstanceForEntity(this);
    }
}

} // namespace SVE