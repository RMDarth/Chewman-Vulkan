// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0#include "MaterialManager.h"
#include "MaterialManager.h"
#include "VulkanException.h"
namespace SVE
{

void MaterialManager::registerMaterial(std::shared_ptr<Material> material)
{
    _materialMap.insert({material->getName(), material});
}

std::shared_ptr<Material> SVE::MaterialManager::getMaterial(const std::string& name) const
{
    auto materialIter = _materialMap.find(name);
    if (materialIter == _materialMap.end())
    {
        return nullptr;
    }
    return materialIter->second;
}

void MaterialManager::resetPipelines()
{
    for (auto& material : _materialMap)
    {
        material.second->resetPipeline();
    }
}

void MaterialManager::resetDescriptors()
{
    for (auto& material : _materialMap)
    {
        material.second->resetDescriptorSets();
    }
}


} // namespace SVE