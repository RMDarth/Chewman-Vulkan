#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightDirectViewProjection;
    vec4 clipPlane;
} uniforms;

layout(set = 0, binding = 1) readonly buffer SSBO
{
    mat4 modelList[];
} ssbo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inBinormal;
layout (location = 5) in vec3 inTangent;


layout(location = 0) out OutData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragBinormal;
    vec3 fragTangent;
    vec3 fragPos;
    vec4 fragDirectLightSpacePos;
};

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[];
};

void main() {
    mat4 model = ssbo.modelList[gl_InstanceIndex];
    vec4 worldPos = model * vec4(inPosition, 1.0);
    vec4 camPos = uniforms.view * worldPos;

    gl_Position = uniforms.projection * camPos;
    gl_ClipDistance[0] = dot(worldPos, uniforms.clipPlane);
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    fragPos = vec3(worldPos);
    mat4 invWMap = transpose(inverse(model));  // transpose(inverse(uniforms.model)) not working for assimp
    fragNormal = vec3(invWMap * vec4(inNormal.xyz, 1.0));
    fragBinormal = vec3(invWMap * vec4(inBinormal.xyz, 1.0));
    fragTangent = vec3(invWMap * vec4(inTangent.xyz, 1.0));

    fragDirectLightSpacePos =  uniforms.lightDirectViewProjection * worldPos;
    fragDirectLightSpacePos.xyzw = fragDirectLightSpacePos.xyzw / fragDirectLightSpacePos.w;
}