#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_BONES 64
layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	vec4 clipPlane;
	mat4 bones[MAX_BONES];
} uniforms;

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
    float gl_ClipDistance[];
};

void main() {
    mat4 boneTransform = uniforms.bones[inBoneIDs[0]] * inBoneWeights[0];
    boneTransform     += uniforms.bones[inBoneIDs[1]] * inBoneWeights[1];
    boneTransform     += uniforms.bones[inBoneIDs[2]] * inBoneWeights[2];
    boneTransform     += uniforms.bones[inBoneIDs[3]] * inBoneWeights[3];

    vec4 worldPos = uniforms.model * boneTransform * vec4(inPosition, 1.0);

    gl_Position = uniforms.projection * uniforms.view * worldPos;
    gl_ClipDistance[0] = dot(worldPos, uniforms.clipPlane);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(worldPos);
    fragNormal = mat3(inverse(transpose(uniforms.model * boneTransform))) * inNormal;
}