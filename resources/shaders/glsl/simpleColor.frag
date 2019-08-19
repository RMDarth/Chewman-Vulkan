#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    MaterialInfo materialInfo;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 diffuse = texture(texSampler, fragTexCoord).rgba;

    vec3 result = diffuse.rgb * ubo.materialInfo.diffuse.rgb * fragColor;
    outColor = vec4(result, diffuse.a * ubo.materialInfo.diffuse.a);
}