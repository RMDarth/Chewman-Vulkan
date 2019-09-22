// SVE (Simple Vulkan Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include "OverlaySettings.h"
#include "OverlayEntity.h"

namespace SVE
{

class OverlayManager
{
public:
    void addOverlay(OverlayInfo info);
    void addOverlay(std::shared_ptr<OverlayEntity> entity);
    void removeOverlay(const std::string& name);
    void changeOverlayOrder(const std::string& name, uint32_t newOrder);

    void updateUniforms(UniformDataList uniformDataList) const;
    void applyDrawingCommands(uint32_t bufferIndex, uint32_t imageIndex) const;

private:
    std::unordered_map<std::string, std::shared_ptr<OverlayEntity>> _overlayList;
    std::map<uint32_t, std::vector<std::shared_ptr<OverlayEntity>>> _overlayZMap;
};

} // namespace SVE