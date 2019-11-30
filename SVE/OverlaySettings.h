// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "TextSettings.h"

namespace SVE
{

struct OverlayInfo
{
    std::string name;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    std::string materialName;
    uint32_t zOrder = 100;

    glm::vec4 texCoord = { 0.0f, 1.0f, 0.0f, 1.0f }; // minX, maxX, minY, maxY

    TextInfo textInfo {};
    TextAlignment textHAlignment = TextAlignment::Center;
    TextVerticalAlignment textVAlignment = TextVerticalAlignment::Center;
};

struct UniformOverlayInfo
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    glm::vec4 texCoord = {0.0f, 1.0f, 0.0f, 1.0f};
};

} // namespace SVE