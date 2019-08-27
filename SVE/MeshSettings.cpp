// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License

#include "MeshSettings.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SVE
{

namespace
{

const aiNodeAnim* findAnimNode(const aiAnimation* animation, const std::string nodeName)
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
aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* animNode)
{
    aiVector3D translation;

    if (animNode->mNumPositionKeys == 1)
    {
        translation = animNode->mPositionKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < animNode->mNumPositionKeys - 1; i++)
        {
            if (time < (float)animNode->mPositionKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = animNode->mPositionKeys[frameIndex];
        aiVectorKey nextFrame = animNode->mPositionKeys[(frameIndex + 1) % animNode->mNumPositionKeys];

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
aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* animNode)
{
    aiQuaternion rotation;

    if (animNode->mNumRotationKeys == 1)
    {
        rotation = animNode->mRotationKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < animNode->mNumRotationKeys - 1; i++)
        {
            if (time < (float)animNode->mRotationKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiQuatKey currentFrame = animNode->mRotationKeys[frameIndex];
        aiQuatKey nextFrame = animNode->mRotationKeys[(frameIndex + 1) % animNode->mNumRotationKeys];

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
aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* animNode)
{
    aiVector3D scale;

    if (animNode->mNumScalingKeys == 1)
    {
        scale = animNode->mScalingKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < animNode->mNumScalingKeys - 1; i++)
        {
            if (time < (float)animNode->mScalingKeys[i + 1].mTime)
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = animNode->mScalingKeys[frameIndex];
        aiVectorKey nextFrame = animNode->mScalingKeys[(frameIndex + 1) % animNode->mNumScalingKeys];

        float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

        const aiVector3D& start = currentFrame.mValue;
        const aiVector3D& end = nextFrame.mValue;

        scale = (start + delta * (end - start));
    }

    aiMatrix4x4 mat;
    aiMatrix4x4::Scaling(scale, mat);
    return mat;
}

void iterateBones(
        std::vector<glm::mat4>& boneData,
        float time,
        const std::shared_ptr<AnimationSettings>& animationSettings,
        const uint32_t animationId,
        const aiNode* node,
        const aiMatrix4x4& parentTransform)
{
    std::string nodeName(node->mName.data);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    const auto* animNode = findAnimNode(animationSettings->animations[animationId], nodeName);

    if (animNode)
    {
        // Get interpolated matrices between current and next frame
        aiMatrix4x4 matScale = interpolateScale(time, animNode);
        aiMatrix4x4 matRotation = interpolateRotation(time, animNode);
        aiMatrix4x4 matTranslation = interpolateTranslation(time, animNode);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;

    if (animationSettings->boneMap.find(nodeName) != animationSettings->boneMap.end())
    {
        uint32_t boneIndex = animationSettings->boneMap.at(nodeName);
        if (boneIndex < boneData.size() && boneIndex >= 0)
        {
            auto finalTransform =
                    animationSettings->globalInverse * globalTransformation * animationSettings->boneOffset[boneIndex];
            boneData[boneIndex] = glm::transpose(glm::make_mat4(&finalTransform.a1));
        }
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        iterateBones(boneData, time, animationSettings, animationId, node->mChildren[i], globalTransformation);
    }
}

} // anon namespace


std::vector<glm::mat4> getAnimationTransforms(const MeshSettings& meshSettings, uint32_t animationId, float time)
{
    // TODO: Only for looped anims
    auto duration = meshSettings.animation->animations[animationId]->mDuration;
    if (duration > 0)
    {
        while (time > duration)
        {
            time -= duration;
        }
    }

    std::vector<glm::mat4> result(meshSettings.boneNum, glm::mat4(1));
    iterateBones(result, time, meshSettings.animation, animationId, meshSettings.animation->rootNode, aiMatrix4x4());

    return result;
}
} // namespace SVE