// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Entity.h"
#include "MeshSettings.h"
#include <memory>

namespace SVE
{
class VulkanMesh;

class MeshEntity : public Entity
{
public:
    // TODO: Remove this constructor, loading models should be done by special managers or functions
    explicit MeshEntity(std::string modelFile);
    explicit MeshEntity(MeshSettings meshSettings);
    ~MeshEntity();

    SubmitInfo render() const override;

private:
    std::unique_ptr<VulkanMesh> _vulkanMesh;
};

} // namespace SVE