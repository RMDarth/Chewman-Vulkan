#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 1) uniform UBO
{
    MaterialInfo materialInfo;
} ubo;

layout(location = 0) in FragData
{
    vec2 fragTexCoord;
    vec4 fragColor;
    float fragLifePercent;
};

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 result = texture(texSampler, fragTexCoord) * ubo.materialInfo.diffuse;
    outColor = vec4(vec4(result.rgb * 10.0, result.a) + fragColor);
}
