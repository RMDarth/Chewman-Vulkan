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
    void updateUniformDataBones(UniformData& data, float time);

    void subscribeToAttachment(const std::string& name);
    void unsubscribeFromAttachment(const std::string& name);
    glm::mat4 getAttachment(const std::string& name);

private:
    std::string _name;
    std::string _materialName;

    BonesAttachments _attachments;

    bool _isAnimated;

    std::unique_ptr<VulkanMesh> _vulkanMesh;
};

} // namespace SVE