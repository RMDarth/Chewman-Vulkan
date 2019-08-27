// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

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

void Material::resetPipeline()
{
    _vulkanMaterial->resetPipeline();
}

void Material::resetDescriptorSets()
{
    _vulkanMaterial->resetDescriptorSets();
}


} // namespace SVE