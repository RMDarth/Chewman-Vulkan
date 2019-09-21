// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "SceneNode.h"
#include "TextSettings.h"
#include "Material.h"
#include "Mesh.h"

namespace SVE
{

class TextEntity : public Entity
{
public:
    explicit TextEntity(TextInfo textInfo);
    ~TextEntity() override;

    TextInfo& getText();
    void setText(TextInfo textInfo);

    void updateUniforms(UniformDataList uniformDataList) const override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;

private:
    TextInfo _textInfo;
    Material* _material;
    uint32_t _materialIndex;
};

} // namespace SVE