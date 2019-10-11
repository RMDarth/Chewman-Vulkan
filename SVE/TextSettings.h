// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <cstdint>
#include <vector>
#include <string>
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
    uint32_t size;
    int32_t maxHeight;
};

struct TextSymbolInfo
{
    uint32_t symbolInfoIndex;
    uint32_t x;
    uint32_t y;
    uint32_t _padding;
};

enum class TextAlignment
{
    Left,
    Center,
    Right
};

enum class TextVerticalAlignment
{
    Top,
    Center,
    Bottom
};

struct TextInfo
{
    std::string text;
    Font* font = nullptr;
    uint32_t symbolCount = 0;
    std::vector<TextSymbolInfo> symbols;
    glm::ivec2 textSize;
    float scale = 1.0f;
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct UniformTextInfo
{
    glm::ivec2 fontImageSize;
    glm::vec2 imageSize;
    glm::vec4 color;
    uint32_t symbolCount;
    uint32_t maxHeight;
    float scale;
    uint32_t _padding[1];
};

} // namespace SVE