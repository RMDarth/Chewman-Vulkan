// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace SVE
{

struct GlyphInfo
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    int32_t originX;
    int32_t originY;
    uint32_t advance;
    uint32_t _padding;
};

struct Font
{
    std::string fontName;
    GlyphInfo symbols[300];
    std::unordered_map<char, uint32_t> symbolToInfoPos;
    std::string materialName;
    uint32_t width;
    uint32_t height;
    int32_t maxHeight;
};

struct TextSymbolInfo
{
    uint32_t symbolInfoIndex;
    uint32_t x;
    uint32_t y;
    uint32_t _padding;
};

struct TextInfo
{
    Font* font;
    uint32_t symbolCount;
    std::vector<TextSymbolInfo> symbols;
};

struct UniformTextInfo
{
    glm::ivec2 fontImageSize;
    glm::vec2 imageSize;
    uint32_t symbolCount;
    uint32_t maxHeight;
    uint32_t _padding[2];
};

} // namespace SVE