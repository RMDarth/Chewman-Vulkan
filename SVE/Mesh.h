// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "MeshSettings.h"
#include <memory>

namespace SVE
{
class VulkanMesh;

class Mesh
{
public:
    explicit Mesh(MeshSettings meshSettings);
    explicit Mesh(const std::string& name, const std::string& modelFile);
    ~Mesh();

    const std::string& getName();
    const std::string& getDefaultMaterialName();
    VulkanMesh* getVulkanMesh();

private:
    std::string _name;
    std::string _materialName;
    std::unique_ptr<VulkanMesh> _vulkanMesh;
};

} // namespace SVE