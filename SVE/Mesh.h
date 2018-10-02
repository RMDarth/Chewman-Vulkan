// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "MeshSettings.h"
#include <memory>

namespace SVE
{
class VulkanMesh;
class UniformData;

class Mesh
{
public:
    explicit Mesh(MeshSettings meshSettings);
    explicit Mesh(const std::string& name, const std::string& modelFile);
    ~Mesh();

    const std::string& getName() const;
    const std::string& getDefaultMaterialName() const;
    VulkanMesh* getVulkanMesh();

    // TODO: this should be moved to something like Animation class
    void updateUniformDataBones(UniformData& data, float time) const;

private:
    std::string _name;
    std::string _materialName;

    bool _isAnimated;

    std::unique_ptr<VulkanMesh> _vulkanMesh;
};

} // namespace SVE