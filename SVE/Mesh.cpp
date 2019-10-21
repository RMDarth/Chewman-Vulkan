// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "Mesh.h"
#include "VulkanMesh.h"
#include "VulkanException.h"
#include "ShaderSettings.h"
#include "Engine.h"
#include "ResourceManager.h"

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
    , _isAnimated(meshSettings.boneNum > 0 && meshSettings.animation->animations != nullptr)
    , _vulkanMesh(std::make_unique<VulkanMesh>(std::move(meshSettings)))
{

}

Mesh::Mesh(MeshLoadSettings meshLoadSettings)
    : _name(meshLoadSettings.name)
{
    MeshSettings meshSettings {};

    //Assimp::Importer importer;
    meshSettings.animation = std::make_shared<AnimationSettings>();
    meshSettings.animationSpeed = meshLoadSettings.animationSpeed;
    auto& importer = meshSettings.animation->importer;

    std::map<std::string, uint32_t> boneMap;

    auto fileContent = Engine::getInstance()->getResourceManager()->loadFileContent(meshLoadSettings.filename);
    const aiScene* scene = importer.ReadFileFromMemory(
            fileContent.data(), fileContent.size(),
            aiProcess_CalcTangentSpace       |
            aiProcess_Triangulate            |
            aiProcess_JoinIdenticalVertices  |
            aiProcess_TransformUVCoords      |
            aiProcess_LimitBoneWeights       |
            aiProcess_OptimizeMeshes         |
            aiProcess_SortByPType            |
            aiProcess_OptimizeGraph);

    // If the import failed, report it
    if(!scene)
    {
        throw VulkanException( importer.GetErrorString());
    }

    meshSettings.name = meshLoadSettings.name;
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
        meshSettings.vertexBinormalData.reserve(mesh->mNumVertices);
        meshSettings.vertexTangentData.reserve(mesh->mNumVertices);
        for (auto v = 0u; v < mesh->mNumVertices; v++)
        {
            if (meshLoadSettings.switchYZ)
            {
                meshSettings.vertexPosData.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].z, mesh->mVertices[v].y);
                meshSettings.vertexPosData.back() *= meshLoadSettings.scale;
                meshSettings.vertexColorData.emplace_back(1.0f, 1.0f, 1.0f);
                meshSettings.vertexTexData.emplace_back(mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y);
                meshSettings.vertexNormalData.emplace_back(mesh->mNormals[v].x, mesh->mNormals[v].z, mesh->mNormals[v].y);
                meshSettings.vertexBinormalData.emplace_back(mesh->mBitangents[v].x, mesh->mBitangents[v].z, mesh->mBitangents[v].y);
                meshSettings.vertexTangentData.emplace_back(mesh->mTangents[v].x, mesh->mTangents[v].z, mesh->mTangents[v].y);
            } else {
                meshSettings.vertexPosData.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
                meshSettings.vertexPosData.back() *= meshLoadSettings.scale;
                meshSettings.vertexColorData.emplace_back(1.0f, 1.0f, 1.0f);
                meshSettings.vertexTexData.emplace_back(mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y);
                meshSettings.vertexNormalData.emplace_back(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
                meshSettings.vertexBinormalData.emplace_back(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
                meshSettings.vertexTangentData.emplace_back(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
            }
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


    _isAnimated = false;
    if (scene->mNumAnimations > 0)
    {
        meshSettings.animation->animations = scene->mAnimations;
        meshSettings.animation->rootNode = scene->mRootNode;
        meshSettings.animation->globalInverse = scene->mRootNode->mTransformation;
        meshSettings.animation->globalInverse.Inverse();
        meshSettings.animation->boneMap = boneMap;
        _isAnimated = true;
    }


    _vulkanMesh = std::make_unique<VulkanMesh>(std::move(meshSettings));
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

void Mesh::updateMesh(MeshSettings meshSettings)
{
    _vulkanMesh->updateMesh(std::move(meshSettings));
}

void Mesh::updateUniformDataBones(UniformData& data, float time)
{
    if (_isAnimated)
        data.bones = getAnimationTransforms(_vulkanMesh->getMeshSettings(), 0, time, _attachments);
}

void Mesh::subscribeToAttachment(const std::string& name)
{
    _attachments[name] = glm::mat4(1);
}

void Mesh::unsubscribeFromAttachment(const std::string& name)
{
    _attachments.erase(name);
}

glm::mat4 Mesh::getAttachment(const std::string& name)
{
    assert(_attachments.find(name) != _attachments.end());
    return _attachments.at(name);
}

} // namespace SVE