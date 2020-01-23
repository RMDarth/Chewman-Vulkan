#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D smokeSampler;

layout(set = 1, binding = 1) uniform UBO
{
    vec4 cameraPos;
    float width;
} ubo;

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
    vec2 texCoord = fragTexCoord * 10;
    vec3 color = texture(smokeSampler, texCoord, 0).rgb;

    vec3 cameraDir = fragPos - ubo.cameraPos.xyz;
    float len1 = length(cameraDir);
    cameraDir = normalize(cameraDir);
    vec3 toWall = vec3(0, 0, 1);
    float wallCamera = dot(toWall, cameraDir);
    float depthDiff = 1.0;
    if (wallCamera != 0)
    {
        float len2 = (-ubo.cameraPos.z + 1.5) / wallCamera;

        depthDiff = (len2 - len1) * 0.5;
        depthDiff = mix(depthDiff, 1.0, smoothstep(-1.5, -2, fragPos.x));
        depthDiff = mix(depthDiff, 1.0, smoothstep(ubo.width - 1.5, ubo.width-1.0, fragPos.x));
        depthDiff = mix(depthDiff, 1.0, smoothstep(0, -10, fragPos.z));
    }

    depthDiff = depthDiff < 0 ? 1 : depthDiff;
    outColor = vec4(color, depthDiff);
    outColorBloom = vec4(outColor.rgb, 0.35 * min(depthDiff, 1.0));
}