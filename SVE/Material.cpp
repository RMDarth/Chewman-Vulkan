// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "Material.h"
#include "VulkanMaterial.h"

namespace SVE
{

Material::Material(MaterialSettings materialSettings)
    : _name(materialSettings.name)
    , _vulkanMaterial(std::make_unique<VulkanMaterial>(std::move(materialSettings)))
{

}

Material::~Material() = default;

VulkanMaterial* Material::getVulkanMaterial()
{
    return _vulkanMaterial.get();
}

const std::string &Material::getName()
{
    return _name;
}




} // namespace SVE