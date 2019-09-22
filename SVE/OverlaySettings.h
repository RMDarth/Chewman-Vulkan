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

    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    std::string materialName;
    uint32_t zOrder = 100;

    TextInfo textInfo {};
    TextAlignment textHAlignment = TextAlignment::Center;
    TextVerticalAlignment textVAlignment = TextVerticalAlignment::Center;
};

struct UniformOverlayInfo
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

} // namespace SVE