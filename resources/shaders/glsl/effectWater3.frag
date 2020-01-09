// based on https://www.shadertoy.com/view/lltXD4
#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D mainSampler;
layout(set = 1, binding = 1) uniform sampler2D dudvSampler;
layout(set = 1, binding = 2) uniform UBO
{
    vec4 cameraPos;
    DirLight dirLight;
    MaterialInfo materialInfo;
    float time;
} ubo;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragBinormal;
layout (location = 4) in vec3 fragTangent;
layout (location = 5) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

const float waveStrength = 0.005; // 0.02

vec3 getColor()
{
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

    vec3 lightEffect = CalcDirLight(ubo.dirLight, norm, viewDir, ubo.materialInfo);
    vec3 finalLight = mix(lightEffect * 4, vec3(1,1,1), step(0.5, ubo.dirLight.diffuse.r));

    vec2 reflectionCoord = fragTexCoord * 3;

    float timeMult = ubo.time * 0.05;
    float clampTime = timeMult - floor(timeMult);
    vec2 dudvCoords =  fragTexCoord * 5;
    vec2 distortion1 = (texture(dudvSampler, vec2(dudvCoords.x + clampTime, dudvCoords.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(dudvSampler, vec2(-dudvCoords.x + clampTime, dudvCoords.y + clampTime)).rg * 2.0 - 1.0) * waveStrength;
    vec2 totalDistortion = distortion1 + distortion2;

    reflectionCoord = reflectionCoord + totalDistortion;
    vec3 result = mix(texture(mainSampler, reflectionCoord).rgb, vec3(0.0, 0.3, 0.5), 0.4);

    return vec3(finalLight * mix(result, vec3(0.0, 0.3, 0.5), 0.3));
}

void main() {
    vec3 color = getColor();

    outColor = vec4(color, 1.0);
}