// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
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
    explicit Mesh(MeshLoadSettings meshLoadSettings);
    ~Mesh();

    const std::string& getName() const;
    const std::string& getDefaultMaterialName() const;
    VulkanMesh* getVulkanMesh();

    void updateMesh(MeshSettings meshSettings);

    // TODO: this should be moved to something like Animation class
    void updateUniformDataBones(UniformData& data, float time, BonesAttachments& bonesAttachments);

private:
    std::string _name;
    std::string _materialName;

    bool _isAnimated;

    std::unique_ptr<VulkanMesh> _vulkanMesh;
};

} // namespace SVE