// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "OverlaySettings.h"
#include "Material.h"
#include "Mesh.h"

#include <set>

namespace SVE
{

class OverlayEntity : public Entity
{
public:
    explicit OverlayEntity(OverlayInfo overlayInfo);
    ~OverlayEntity() override;

    OverlayInfo& getInfo();
    void setText(TextInfo textInfo);

    void setMaterial(const std::string& materialName) override;

    void setVisible(bool visible);

    void updateUniforms(UniformDataList uniformDataList) const override;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const override;

private:
    void initText();

private:
    OverlayInfo _overlayInfo;
    Material* _material = nullptr;
    std::set<Material*> _materialList;
    uint32_t _materialIndex = 0;

    Material* _textMaterial = nullptr;
    uint32_t _textMaterialIndex = 0;

    bool _isVisible = true;
};

} // namespace SVE