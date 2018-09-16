// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#pragma once
#include <string>
#include <memory>
#include "MaterialSettings.h"

namespace SVE
{
class VulkanMaterial;

class Material
{
public:
    explicit Material(MaterialSettings materialSettings);
    ~Material();

    const std::string& getName();
    VulkanMaterial* getVulkanMaterial();
    void resetPipeline();

private:
    std::string _name;
    std::unique_ptr<VulkanMaterial> _vulkanMaterial;
};

} // namespace SVE