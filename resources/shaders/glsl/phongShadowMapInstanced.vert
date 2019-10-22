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

layout(set = 0, binding = 1) buffer SSBO
{
    mat4 modelList[];
} ssbo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout(location = 0) out OutData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragPos;
    vec4 fragDirectLightSpacePos;
};

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 worldPos = ssbo.modelList[gl_InstanceIndex] * vec4(inPosition, 1.0);
    vec4 camPos = uniforms.view * worldPos;

    gl_Position = uniforms.projection * camPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    fragPos = vec3(worldPos);
    fragNormal = vec3(transpose(inverse(ssbo.modelList[gl_InstanceIndex])) * vec4(inNormal.xyz, 1.0));

    fragDirectLightSpacePos =  uniforms.lightDirectViewProjection * worldPos;
    fragDirectLightSpacePos.xyzw = fragDirectLightSpacePos.xyzw / fragDirectLightSpacePos.w;
}