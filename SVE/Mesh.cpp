// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Mesh.h"
#include "VulkanMesh.h"
#include "VulkanException.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SVE
{

Mesh::Mesh(MeshSettings meshSettings)
    : _name(meshSettings.name)
    , _materialName(meshSettings.materialName)
    , _vulkanMesh(std::make_unique<VulkanMesh>(std::move(meshSettings)))
{

}

Mesh::Mesh(const std::string& name, const std::string& modelFile)
    : _name(name)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
            modelFile,
            aiProcess_CalcTangentSpace       |
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_SortByPType);

    // If the import failed, report it
    if(!scene)
    {
        throw VulkanException( importer.GetErrorString());
    }

    MeshSettings meshSettings {};

    meshSettings.name = name;
    aiString materialName;
    scene->mMaterials[0]->Get(AI_MATKEY_NAME, materialName);

    meshSettings.materialName = materialName.C_Str();
    _materialName = meshSettings.materialName;
    for (auto i = 0u; i < scene->mNumMeshes; i++)
    {
        const auto* mesh = scene->mMeshes[i];
        meshSettings.vertexPosData.reserve(meshSettings.vertexPosData.size() + mesh->mNumVertices);
        meshSettings.vertexColorData.reserve(meshSettings.vertexColorData.size() + mesh->mNumVertices);
        meshSettings.vertexTexData.reserve(meshSettings.vertexTexData.size() + mesh->mNumVertices);
        for (auto v = 0u; v < mesh->mNumVertices; v++)
        {
            meshSettings.vertexPosData.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            meshSettings.vertexColorData.emplace_back(1.0f, 1.0f, 1.0f);
            meshSettings.vertexTexData.emplace_back(mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y);

        }
        meshSettings.indexData.reserve(meshSettings.indexData.size() + mesh->mNumFaces * 3);
        for (auto f = 0u; f < mesh->mNumFaces; f++)
        {
            for (auto index = 0u; index < mesh->mFaces[f].mNumIndices; index++)
            {
                meshSettings.indexData.push_back(mesh->mFaces[f].mIndices[index]);
            }
        }
    }

    _vulkanMesh = std::make_unique<VulkanMesh>(meshSettings);
}

Mesh::~Mesh() = default;

const std::string& Mesh::getName()
{
    return _name;
}

const std::string& Mesh::getDefaultMaterialName()
{
    return _materialName;
}

VulkanMesh* Mesh::getVulkanMesh()
{
    return _vulkanMesh.get();
}

} // namespace SVE