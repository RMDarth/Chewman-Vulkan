// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include "MaterialManager.h"
#include "VulkanException.h"
namespace SVE
{

void MaterialManager::registerMaterial(std::shared_ptr<Material> material)
{
    _materialMap.insert({material->getName(), material});
}

Material* SVE::MaterialManager::getMaterial(const std::string& name, bool emptyAllowed) const
{
    auto materialIter = _materialMap.find(name);
    if (materialIter == _materialMap.end())
    {
        if (!emptyAllowed)
            throw VulkanException(std::string("Can't find material ") + name);
        return nullptr;
    }
    return materialIter->second.get();
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