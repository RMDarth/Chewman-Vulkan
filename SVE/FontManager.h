// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "TextSettings.h"
#include "ShaderSettings.h"
#include <memory>
#include <glm/glm.hpp>

namespace SVE
{

class FontManager
{
public:
    TextInfo generateText(const std::string& text, const std::string& font, float scale = 1.0f, glm::ivec2 shift = {0, 0});
    void addFont(Font font);

    // TODO: move this to Overlay manager
    //void fillUniformData(UniformData& data);

private:
    std::unordered_map<std::string, Font> _fontList;
};

} // namespace SVE