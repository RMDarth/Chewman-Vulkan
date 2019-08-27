// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#pragma once
#include <memory>
#include <unordered_map>
#include "Mesh.h"

namespace SVE
{

class MeshManager
{
public:
    void registerMesh(std::shared_ptr<Mesh> mesh);
    Mesh* getMesh(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> _meshMap;
};

} // namespace SVE