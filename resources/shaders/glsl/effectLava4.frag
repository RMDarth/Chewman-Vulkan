#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D lavaSampler;
layout(set = 1, binding = 1) uniform UBO
{
    float time;
} ubo;

layout (location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    vec2 texCoord = fragTexCoord * 2;
    float time = ubo.time * 0.005;
    texCoord.x += time;
    texCoord.y += time * 0.5;
    vec3 color = texture(lavaSampler, texCoord, 0).rgb;

    outColor = vec4(color, 1.0);
}