#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_BONES 64
layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 bones[MAX_BONES];
} matrices;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec4 inBoneWeights;
layout (location = 5) in ivec4 inBoneIDs;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 boneTransform = matrices.bones[inBoneIDs[0]] * inBoneWeights[0];
    boneTransform     += matrices.bones[inBoneIDs[1]] * inBoneWeights[1];
    boneTransform     += matrices.bones[inBoneIDs[2]] * inBoneWeights[2];
    boneTransform     += matrices.bones[inBoneIDs[3]] * inBoneWeights[3];

    gl_Position = matrices.projection * matrices.view * matrices.model * boneTransform * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(matrices.model * boneTransform * vec4(inPosition, 1.0));
    fragNormal = mat3(inverse(transpose(matrices.model * boneTransform))) * inNormal;
}