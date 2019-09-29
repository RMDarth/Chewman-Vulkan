#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    MaterialInfo materialInfo;
    float time;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorBloom;

void main() {
    vec2 texCoord = vec2(fragTexCoord.x, mod(fragTexCoord.y - ubo.time * 0.7, 1.0));
    vec4 diffuse = texture(texSampler, texCoord).rgba;

    vec3 result = diffuse.rgb * ubo.materialInfo.diffuse.rgb * fragColor;
    outColor = vec4(result, diffuse.a * ubo.materialInfo.diffuse.a);
    outColorBloom = vec4(outColor.rgb, 0.3);
}