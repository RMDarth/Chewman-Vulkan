// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#pragma once
#include "Libs.h"
#include <string>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include <memory>

// TODO: Revise
#include <assimp/Importer.hpp>
#include <assimp/scene.h>


namespace SVE
{

template <typename T>
using AnimData = std::vector<T>;

struct BoneInfo
{
    int boneId;

    AnimData<glm::vec3> positionData;
    AnimData<float> positionTime;
    AnimData<glm::quat> rotationData;
    AnimData<float> rotationTime;
    AnimData<glm::vec3> scaleData;
    AnimData<float> scaleTime;
    glm::mat4 transform;

    std::vector<std::shared_ptr<BoneInfo>> children;
};

struct AnimationSettings
{
    float duration;

    std::shared_ptr<BoneInfo> rootNodeAnimation;
};

struct AnimationSettings2
{
    float duration;

    Assimp::Importer importer;
    aiAnimation* animation;
    aiNode* rootNode;
    std::vector<aiMatrix4x4> boneOffset;
    aiMatrix4x4 globalInverse;
    std::map<std::string, uint32_t> boneMap;
};


struct MeshSettings
{
    std::string name;
    std::vector<glm::vec3> vertexPosData;
    std::vector<glm::vec3> vertexColorData;
    std::vector<glm::vec2> vertexTexData;
    std::vector<glm::vec3> vertexNormalData;
    std::vector<uint32_t> indexData;

    uint32_t boneNum;
    std::vector<glm::ivec4> vertexBoneIndexData;
    std::vector<glm::vec4> vertexBoneWeightData;
    std::vector<glm::mat4> boneOffset;
    glm::mat4 globalInverse;
    AnimationSettings animation;
    std::shared_ptr<AnimationSettings2> animation2;

    std::string materialName;
};

std::vector<glm::mat4> getTransforms(const MeshSettings& meshSettings, const AnimationSettings& settings, float time);

std::vector<glm::mat4> getTransforms2(const MeshSettings& meshSettings, float time);

} // namespace SVE