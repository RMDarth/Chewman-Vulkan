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
    meshSettings.animation2 = std::make_shared<AnimationSettings2>();
    auto& importer = meshSettings.animation2->importer;

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
            meshSettings.boneOffset.resize(mesh->mNumBones);
            meshSettings.animation2->boneOffset.resize(mesh->mNumBones);
            meshSettings.globalInverse = glm::inverse(glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1)));

            for (auto r = 0u; r < mesh->mNumBones; r++)
            {
                auto* boneInfo = mesh->mBones[r];
                boneMap[boneInfo->mName.C_Str()] = r;
                meshSettings.animation2->boneOffset[r] = boneInfo->mOffsetMatrix;
                meshSettings.boneOffset[r] = glm::transpose(glm::make_mat4(&boneInfo->mOffsetMatrix.a1));
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
        // TODO: Support loading several animations
        auto animation = scene->mAnimations[0];
        AnimationSettings animationSettings;

        animationSettings.duration = (float)animation->mDuration;
        animationSettings.rootNodeAnimation = std::make_shared<BoneInfo>();
        std::map<std::string, std::shared_ptr<BoneInfo>> boneInfoMap;

        // go through all bones, set their ids. children and default transform
        std::stack<std::pair<aiNode*, std::shared_ptr<BoneInfo>>> s;
        s.push({scene->mRootNode, animationSettings.rootNodeAnimation});
        boneMap[scene->mRootNode->mName.C_Str()] = meshSettings.boneNum;
        animationSettings.rootNodeAnimation->boneId = meshSettings.boneNum;
        animationSettings.rootNodeAnimation->transform = glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1));
        boneInfoMap[scene->mRootNode->mName.C_Str()] = animationSettings.rootNodeAnimation;
        while (!s.empty())
        {
            auto nodePair = s.top();
            s.pop();
            auto* node = nodePair.first;
            auto boneInfo = nodePair.second;

            for (auto r = 0u; r < node->mNumChildren; r++)
            {
                auto boneName = fixAssimpBoneName(node->mChildren[r]->mName.C_Str());

                auto childInfo = std::make_shared<BoneInfo>();
                childInfo->boneId = boneMap.find(boneName) != boneMap.end() ? boneMap[boneName] : -1;
                if (childInfo->boneId == -1)
                    std::cout << boneName << std::endl;
                childInfo->transform = glm::transpose(glm::make_mat4(&node->mChildren[r]->mTransformation.a1));
                boneInfoMap[node->mChildren[r]->mName.C_Str()] = childInfo;
                boneInfo->children.push_back(childInfo);
                s.push({node->mChildren[r], childInfo});
            }
        }

        int keyPoints = animation->mNumChannels;
        for (auto i = 0u; i < keyPoints; i++)
        {
            auto channel = animation->mChannels[i];
            std::string nodeName = channel->mNodeName.C_Str();

            if (boneInfoMap.find(nodeName) == boneInfoMap.end())
                continue;

            auto boneInfo = boneInfoMap.at(nodeName);

            if (channel->mNumPositionKeys > 0)
            {
                for (auto r = 0u; r < channel->mNumPositionKeys; r++)
                {
                    auto translate = channel->mPositionKeys[r].mValue;
                    boneInfo->positionData.push_back(glm::vec3(translate.x, translate.y, translate.z));
                    boneInfo->positionTime.push_back((float) channel->mPositionKeys[r].mTime);
                }
            }
            if (channel->mNumRotationKeys > 0)
            {
                for (auto r = 0u; r < channel->mNumRotationKeys; r++)
                {
                    auto rotateMatrix = channel->mRotationKeys[r].mValue.GetMatrix();
                    auto glmMatrix = glm::transpose(glm::make_mat3(&rotateMatrix.a1));
                    boneInfo->rotationData.push_back(glm::toQuat(glmMatrix));
                    boneInfo->rotationTime.push_back((float) channel->mRotationKeys[r].mTime);
                }
            }
        }

        meshSettings.animation = animationSettings;

        meshSettings.animation2->animation = scene->mAnimations[0];
        meshSettings.animation2->rootNode = scene->mRootNode;
        meshSettings.animation2->globalInverse = scene->mRootNode->mTransformation;
        meshSettings.animation2->globalInverse.Inverse();
        meshSettings.animation2->boneMap = boneMap;
        meshSettings.animation2->duration = (float)animation->mDuration;
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
    data.bones = getTransforms2(_meshSettings, time);
}

} // namespace SVE