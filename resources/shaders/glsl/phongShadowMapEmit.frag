#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D emitTex;
layout(set = 1, binding = 2) uniform sampler2D directShadowTex;
layout(set = 1, binding = 3) uniform UBO
{
	vec4 cameraPos;
	DirLight dirLight;
	SpotLight spotLight;
    LineLight lineLight[15];
    PointLight pointLight[20];
	LightInfo lightInfo;
	MaterialInfo materialInfo;
} ubo;

layout(location = 0) in InData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragPos;
    vec4 fragDirectLightSpacePos;
};

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorBloom;

#include "lightCalculation.glsl"

void main()
{
    vec3 diffuse = texture(diffuseTex, fragTexCoord).rgb;

    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

    vec3 lightEffect = calculateLight(normal, viewDir);
    vec3 emitEffect = texture(emitTex, fragTexCoord).rgb;
    vec3 result = diffuse * lightEffect * fragColor;
    result = max(result, emitEffect);
    //vec3 result = vec3(shadow);
    outColor = vec4(result, 1.0);
    outColorBloom = vec4(emitEffect, 0.1);
}