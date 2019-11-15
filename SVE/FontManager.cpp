// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "FontManager.h"
#include "VulkanException.h"

namespace SVE
{

void FontManager::addFont(Font font)
{
    _fontList[font.fontName] = std::move(font);
}

TextInfo FontManager::generateText(const std::string& text, const std::string& font, float scale, glm::ivec2 shift, glm::vec4 color)
{
    TextInfo info {};
    info.text = text;
    auto fontIter = _fontList.find(font);
    if (fontIter == _fontList.end())
    {
        // TODO: Rename or add SVE Exception?
        throw VulkanException("Can't find font: " + font + ".");
    }
    info.font = &_fontList.at(font);
    info.symbolCount = text.size();

    glm::vec2 currentShift = shift;
    for (char symbol : text)
    {
        TextSymbolInfo symbolInfo {};
        symbolInfo.symbolInfoIndex = info.font->symbolToInfoPos[symbol];
        symbolInfo.x = currentShift.x;
        symbolInfo.y = currentShift.y;

        auto& glyphInfo = info.font->symbols[symbolInfo.symbolInfoIndex];
        currentShift.x += glyphInfo.advance * scale;
        info.symbols.push_back(symbolInfo);
    }

    info.textSize.x = currentShift.x - shift.x;
    info.textSize.y = info.font->size * scale;
    info.scale = scale;
    info.color = color;
    info.shift = shift;

    return info;
};

} // namespace SVE