// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include <unordered_map>
#include "Material.h"

namespace SVE
{

class MaterialManager
{
public:
    void registerMaterial(std::shared_ptr<Material> material);
    Material* getMaterial(const std::string& name, bool emptyAllowed = false) const;

    void resetPipelines();
    void resetDescriptors();

private:
    std::unordered_map<std::string, std::shared_ptr<Material>> _materialMap;
};

} // namespace SVE