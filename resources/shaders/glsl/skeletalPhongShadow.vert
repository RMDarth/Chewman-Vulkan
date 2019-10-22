#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_BONES 64
layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
    mat4 lightDirectViewProjection;
	vec4 clipPlane;
	mat4 bones[MAX_BONES];
} uniforms;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inBinormal;
layout (location = 5) in vec3 inTangent;
layout (location = 6) in vec4 inBoneWeights;
layout (location = 7) in ivec4 inBoneIDs;

layout(location = 0) out OutData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragPos;
    vec4 fragDirectLightSpacePos;
};

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 boneTransform = uniforms.bones[inBoneIDs[0]] * inBoneWeights[0];
    boneTransform     += uniforms.bones[inBoneIDs[1]] * inBoneWeights[1];
    boneTransform     += uniforms.bones[inBoneIDs[2]] * inBoneWeights[2];
    boneTransform     += uniforms.bones[inBoneIDs[3]] * inBoneWeights[3];

    vec4 worldPos = uniforms.model * boneTransform * vec4(inPosition, 1.0);

    gl_Position = uniforms.projection * uniforms.view * worldPos;

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(worldPos);
    fragNormal = mat3(inverse(transpose(uniforms.model * boneTransform))) * inNormal;

    fragDirectLightSpacePos =  uniforms.lightDirectViewProjection * worldPos;
    fragDirectLightSpacePos.xyzw = fragDirectLightSpacePos.xyzw / fragDirectLightSpacePos.w;
}