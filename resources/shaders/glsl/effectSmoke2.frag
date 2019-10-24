#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D smokeSampler;
layout(set = 1, binding = 1) uniform sampler2D depthSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorBloom;

float linearDepth(float z_b)
{
    float zNear = 0.1 ;
    float zFar = 50.0;

    return (-zFar * zNear / (z_b * (zFar - zNear) - zFar));
}

void main() 
{
    vec2 texCoord = fragTexCoord * 100;
    vec3 color = texture(smokeSampler, texCoord, 0).rgb;

    float depth = textureLod(depthSampler, gl_FragCoord.xy / textureSize(depthSampler, 0), 0).r;
    depth = linearDepth(depth);
    float depthDiff = (depth - gl_FragCoord.z / gl_FragCoord.w) / 3.0;
    outColor = vec4(color, depthDiff);
    outColorBloom = vec4(outColor.rgb, 0.35 * min(depthDiff, 1.0));
}