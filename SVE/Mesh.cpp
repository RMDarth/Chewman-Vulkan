// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Mesh.h"
#include "VulkanMesh.h"
#include "VulkanException.h"
#include "ShaderSettings.h"

#include <stack>
#include <set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SVE
{

namespace
{

std::string fixAssimpBoneName(std::string nodeName)
{
    auto fbxTagPos = nodeName.find("_$AssimpFbx$");
    if (fbxTagPos != std::string::npos)
    {
        nodeName = nodeName.substr(0, fbxTagPos);
    }
    return nodeName;
}

} // anon namespace

Mesh::Mesh(MeshSettings meshSettings)
    : _name(meshSettings.name)
    , _materialName(meshSettings.materialName)
    , _meshSettings(meshSettings)
    , _vulkanMesh(std::make_unique<VulkanMesh>(std::move(meshSettings)))
{

}

Mesh::Mesh(const std::string& name, const std::string& modelFile)
    : _name(name)
{
    MeshSettings meshSettings {};

    //Assimp::Importer importer;
    meshSettings.animation = std::make_shared<AnimationSettings>();
    auto& importer = meshSettings.animation->importer;

    std::map<std::string, uint32_t> boneMap;

    const aiScene* scene = importer.ReadFile(
            modelFile,
            aiProcess_CalcTangentSpace       |
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_LimitBoneWeights       |
            aiProcess_OptimizeGraph);

    // If the import failed, report it
    if(!scene)
    {
        throw VulkanException( importer.GetErrorString());
    }



    meshSettings.name = name;
    aiString materialName;
    scene->mMaterials[0]->Get(AI_MATKEY_NAME, materialName);

    meshSettings.materialName = materialName.C_Str();
    _materialName = meshSettings.materialName;

    // TODO: Support multiple meshes (only 1 currently will work)
    for (auto i = 0u; i < scene->mNumMeshes; i++)
    {
        const auto* mesh = scene->mMeshes[i];
        meshSettings.vertexPosData.reserve(mesh->mNumVertices);
        meshSettings.vertexColorData.reserve(mesh->mNumVertices);
        meshSettings.vertexTexData.reserve(mesh->mNumVertices);
        meshSettings.vertexNormalData.reserve(mesh->mNumVertices);
        for (auto v = 0u; v < mesh->mNumVertices; v++)
        {
            meshSettings.vertexPosData.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            meshSettings.vertexColorData.emplace_back(1.0f, 1.0f, 1.0f);
            meshSettings.vertexTexData.emplace_back(mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y);
            meshSettings.vertexNormalData.emplace_back(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
        }
        meshSettings.indexData.reserve(meshSettings.indexData.size() + mesh->mNumFaces * 3);
        for (auto f = 0u; f < mesh->mNumFaces; f++)
        {
            for (auto index = 0u; index < mesh->mFaces[f].mNumIndices; index++)
            {
                meshSettings.indexData.push_back(mesh->mFaces[f].mIndices[index]);
            }
        }

        if (mesh->mNumBones > 0)
        {
            meshSettings.boneNum = mesh->mNumBones;
            meshSettings.vertexBoneIndexData.resize(mesh->mNumVertices);
            meshSettings.vertexBoneWeightData.resize(mesh->mNumVertices);
            meshSettings.animation->boneOffset.resize(mesh->mNumBones);

            for (auto r = 0u; r < mesh->mNumBones; r++)
            {
                auto* boneInfo = mesh->mBones[r];
                boneMap[boneInfo->mName.C_Str()] = r;
                meshSettings.animation->boneOffset[r] = boneInfo->mOffsetMatrix;
                for (auto w = 0u; w < boneInfo->mNumWeights; w++)
                {
                    auto weight = boneInfo->mWeights[w];

                    auto vID = weight.mVertexId;
                    for (auto bi = 0u; bi < 4; bi++)
                    {
                        if (meshSettings.vertexBoneWeightData[vID][bi] < 0.01f)
                        {
                            meshSettings.vertexBoneIndexData[vID][bi] = r;
                            meshSettings.vertexBoneWeightData[vID][bi] = weight.mWeight;
                            break;
                        }
                    }
                }
            }
        }
    }


    if (scene->mNumAnimations > 0)
    {
        meshSettings.animation->animations = scene->mAnimations;
        meshSettings.animation->rootNode = scene->mRootNode;
        meshSettings.animation->globalInverse = scene->mRootNode->mTransformation;
        meshSettings.animation->globalInverse.Inverse();
        meshSettings.animation->boneMap = boneMap;
    }

    _meshSettings = meshSettings;
    _vulkanMesh = std::make_unique<VulkanMesh>(meshSettings);
}

Mesh::~Mesh() = default;

const std::string& Mesh::getName() const
{
    return _name;
}

const std::string& Mesh::getDefaultMaterialName() const
{
    return _materialName;
}

VulkanMesh* Mesh::getVulkanMesh()
{
    return _vulkanMesh.get();
}

void Mesh::updateUniformDataBones(UniformData& data, float time) const
{
    //data.bones = getTransforms(_meshSettings, _meshSettings.animation, time);
    data.bones = getAnimationTransforms(_meshSettings, 0, time);
}

} // namespace SVE