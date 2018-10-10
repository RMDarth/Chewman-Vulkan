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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace SVE
{

struct AnimationSettings
{
    Assimp::Importer importer;
    aiAnimation** animations;
    aiNode* rootNode;
    std::vector<aiMatrix4x4> boneOffset;
    aiMatrix4x4 globalInverse;
    std::map<std::string, uint32_t> boneMap;
};

struct MeshLoadSettings
{
    std::string name;
    std::string filename;
    bool switchYZ = false;
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
    std::shared_ptr<AnimationSettings> animation;

    std::string materialName;
};

std::vector<glm::mat4> getAnimationTransforms(const MeshSettings& meshSettings, uint32_t animationId, float time);

} // namespace SVE