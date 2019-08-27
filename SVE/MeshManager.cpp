// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "MeshManager.h"

namespace SVE
{

void MeshManager::registerMesh(std::shared_ptr<Mesh> mesh)
{
    _meshMap[mesh->getName()] = std::move(mesh);
}

Mesh* MeshManager::getMesh(const std::string& name) const
{
    auto meshIter = _meshMap.find(name);
    if (meshIter == _meshMap.end())
    {
        return nullptr;
    }
    return meshIter->second.get();
}
} // namespace SVE