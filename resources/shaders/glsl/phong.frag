#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    vec4 cameraPos;
    DirLight dirLight;
    SpotLight spotLight;
    PointLight pointLight[4];
    LightInfo lightInfo;
    MaterialInfo materialInfo;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 diffuse = texture(texSampler, fragTexCoord).rgba;

    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

    vec3 lightEffect = vec3(0);
    if ((ubo.lightInfo.lightFlags & LI_DirectionalLight) != 0)
        lightEffect += CalcDirLight(ubo.dirLight, norm, viewDir, ubo.materialInfo);
    for (uint i = 0; i < 4; i++)
    {
        if ((ubo.lightInfo.lightFlags & LI_PointLight[i]) != 0)
            lightEffect += CalcPointLight(ubo.pointLight[i], norm, fragPos, viewDir, ubo.materialInfo);
    }
    if ((ubo.lightInfo.lightFlags & LI_SpotLight) != 0)
        lightEffect += CalcSpotLight(ubo.spotLight, norm, fragPos, viewDir, ubo.materialInfo);

    vec3 result = diffuse.rgb * lightEffect * fragColor;
    outColor = vec4(result, diffuse.a);
}