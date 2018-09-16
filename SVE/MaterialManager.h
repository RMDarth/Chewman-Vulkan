// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
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
    std::shared_ptr<Material> getMaterial(const std::string& name) const;

    void resetPipelines();

private:
    std::unordered_map<std::string, std::shared_ptr<Material>> _materialMap;
};

} // namespace SVE