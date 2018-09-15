// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0

#include "MeshSettings.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SVE
{

namespace
{

void iterateBones(
        std::vector<glm::mat4>& boneData,
        const MeshSettings& meshSettings,
        const AnimationSettings& settings,
        std::shared_ptr<BoneInfo> boneInfo,
        glm::mat4 parentTransform,
        float time)
{
    glm::mat4 result(1);
    bool changed = false;

    auto nextPosIter = std::upper_bound(boneInfo->positionTime.begin(), boneInfo->positionTime.end(), time);
    if (nextPosIter != boneInfo->positionTime.end() && nextPosIter != boneInfo->positionTime.begin())
    {
        changed = true;
        auto nextPos = std::distance(boneInfo->positionTime.begin(), nextPosIter);
        auto prevPos = nextPos - 1;

        float timeInterpolate = (time - boneInfo->positionTime[prevPos]) /
                                (boneInfo->positionTime[nextPos] - boneInfo->positionTime[prevPos]);
        glm::translate(
                result,
                glm::mix(boneInfo->positionData[prevPos], boneInfo->positionData[nextPos], timeInterpolate));
    }

    // get rotation
    auto nextRotIter = std::upper_bound(boneInfo->rotationTime.begin(), boneInfo->rotationTime.end(), time);
    if (nextRotIter != boneInfo->rotationTime.end() && nextRotIter != boneInfo->rotationTime.begin())
    {
        changed = true;
        auto nextRot = std::distance(boneInfo->rotationTime.begin(), nextRotIter);
        auto prevRot = nextRot - 1;

        float timeInterpolate = (time - boneInfo->rotationTime[prevRot]) /
                                (boneInfo->rotationTime[nextRot] - boneInfo->rotationTime[prevRot]);
        result = result * glm::mat4_cast(
                glm::slerp(boneInfo->rotationData[prevRot], boneInfo->rotationData[nextRot],
                           timeInterpolate)) ;
    }

    //if (!changed)
    {
        result = boneInfo->transform;
    }

    result = result * parentTransform;

    if (boneInfo->boneId < meshSettings.boneNum && boneInfo->boneId >= 0)
    {
        boneData[boneInfo->boneId] = meshSettings.boneOffset[boneInfo->boneId] * result * meshSettings.globalInverse;
    }

    for (const auto& childBone : boneInfo->children)
    {
        iterateBones(boneData, meshSettings, settings, childBone, result, time);
    }
}

const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string nodeName)
{
    for (uint32_t i = 0; i < animation->mNumChannels; i++)
    {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (std::string(nodeAnim->mNodeName.data) == nodeName)
        {
            return nodeAnim;
        }
    }
    return nullptr;
}

// Returns a 4x4 matrix with interpolated translation between current and next frame
aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
{
    aiVector3D translation;

    if (pNodeAnim->mNumPositionKeys == 1)
    {
        translation = pNodeAnim->mPositionKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            if (time < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
        aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];

        float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

        const aiVector3D& start = currentFrame.mValue;
        const aiVector3D& end = nextFrame.mValue;

        translation = (start + delta * (end - start));
    }

    aiMatrix4x4 mat;
    aiMatrix4x4::Translation(translation, mat);
    return mat;
}

// Returns a 4x4 matrix with interpolated rotation between current and next frame
aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim)
{
    aiQuaternion rotation;

    if (pNodeAnim->mNumRotationKeys == 1)
    {
        rotation = pNodeAnim->mRotationKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            if (time < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
        aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];

        float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

        const aiQuaternion& start = currentFrame.mValue;
        const aiQuaternion& end = nextFrame.mValue;

        aiQuaternion::Interpolate(rotation, start, end, delta);
        rotation.Normalize();
    }

    aiMatrix4x4 mat(rotation.GetMatrix());
    return mat;
}


// Returns a 4x4 matrix with interpolated scaling between current and next frame
aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim)
{
    aiVector3D scale;

    if (pNodeAnim->mNumScalingKeys == 1)
    {
        scale = pNodeAnim->mScalingKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
        {
            if (time < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
        aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];

        float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

        const aiVector3D& start = currentFrame.mValue;
        const aiVector3D& end = nextFrame.mValue;

        scale = (start + delta * (end - start));
    }

    aiMatrix4x4 mat;
    aiMatrix4x4::Scaling(scale, mat);
    return mat;
}

void iterateBones2(std::vector<glm::mat4>& boneData, float time, const MeshSettings& meshSettings, const aiNode* pNode, const aiMatrix4x4& ParentTransform)
{
    auto animationSettings = meshSettings.animation2;
    std::string NodeName(pNode->mName.data);

    aiMatrix4x4 NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = findNodeAnim(animationSettings->animation, NodeName);

    if (pNodeAnim)
    {
        // Get interpolated matrices between current and next frame
        aiMatrix4x4 matScale = interpolateScale(time, pNodeAnim);
        aiMatrix4x4 matRotation = interpolateRotation(time, pNodeAnim);
        aiMatrix4x4 matTranslation = interpolateTranslation(time, pNodeAnim);

        NodeTransformation = matTranslation * matRotation * matScale;
    }

    aiMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (animationSettings->boneMap.find(NodeName) != animationSettings->boneMap.end())
    {
        uint32_t BoneIndex = meshSettings.animation2->boneMap.at(NodeName);
        if (BoneIndex < boneData.size() && BoneIndex >= 0)
        {
            auto finalTransform =
                    animationSettings->globalInverse * GlobalTransformation * animationSettings->boneOffset[BoneIndex];
            boneData[BoneIndex] = glm::transpose(glm::make_mat4(&finalTransform.a1));
        }
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; i++)
    {
        iterateBones2(boneData, time, meshSettings, pNode->mChildren[i], GlobalTransformation);
    }
}

} // anon namespace

std::vector<glm::mat4> getTransforms(const MeshSettings& meshSettings, const AnimationSettings& settings, float time)
{
    // TODO: Only for looped anims
    while (time > settings.duration)
    {
        time -= settings.duration;
    }

    std::vector<glm::mat4> result(meshSettings.boneNum, glm::mat4(1));
    iterateBones(result, meshSettings, settings, settings.rootNodeAnimation, glm::mat4(1), time);

    return result;
}

std::vector<glm::mat4> getTransforms2(const MeshSettings& meshSettings, float time)
{
    // TODO: Only for looped anims
    if (meshSettings.animation2->duration > 0)
    {
        while (time > meshSettings.animation2->duration)
        {
            time -= meshSettings.animation2->duration;
        }
    }

    std::vector<glm::mat4> result(meshSettings.boneNum, glm::mat4(1));
    iterateBones2(result, time, meshSettings, meshSettings.animation2->rootNode, aiMatrix4x4());

    return result;
}
} // namespace SVE