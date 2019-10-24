#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D lavaSampler;
layout(set = 1, binding = 1) uniform UBO
{
    float time;
} ubo;

layout(location = 0) in InData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragBinormal;
    vec3 fragTangent;
    vec3 fragPos;
};

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorBloom;


void main() {
    vec2 texCoord = fragTexCoord * 10;
    float time = ubo.time * 0.03;
    texCoord.x += time;
    texCoord.y += time * 0.5;
    vec3 color = texture(lavaSampler, texCoord, 0).rgb;

    float brightness = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    brightness = brightness * step(0.7, brightness);

    outColor = vec4(color, 1.0);
    outColorBloom = vec4(color * brightness * brightness, 0.5);
}