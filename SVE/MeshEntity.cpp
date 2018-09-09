// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "MeshEntity.h"
#include "VulkanMesh.h"
#include "VulkanException.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SVE
{

MeshEntity::MeshEntity(MeshSettings meshSettings)
    : _vulkanMesh(std::make_unique<VulkanMesh>(std::move(meshSettings)))
{

}

MeshEntity::MeshEntity(std::string modelFile)
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

    // TODO: fix it
    aiString name;
    scene->mMaterials[0]->Get(AI_MATKEY_NAME, name);

    meshSettings.materialName = name.C_Str();
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

MeshEntity::~MeshEntity() = default;

SubmitInfo MeshEntity::render() const
{
    _vulkanMesh->updateMatrices(_transformation);
    return SubmitInfo(_vulkanMesh->createSubmitInfo());
}


} // namespace SVE