#version 450
#extension GL_ARB_separate_shader_objects : enable
const uint MAX_LIGHTS = 3;
const uint CASCADE_NUM = 5;
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D emitTex;
layout(set = 1, binding = 2) uniform sampler2DArray directShadowTex;
layout(set = 1, binding = 3) uniform samplerCubeArray pointShadowTex;
layout(set = 1, binding = 4) uniform UBO
{
	vec4 cameraPos;
	DirLight dirLight;
	SpotLight spotLight;
	PointLight shadowPointLight[4];
    LineLight lineLight[15];
    PointLight pointLight[10];
	LightInfo lightInfo;
	MaterialInfo materialInfo;
} ubo;

layout(location = 0) in InData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragPos;
    vec4 fragPointLightSpacePos[MAX_LIGHTS];
    vec4 fragDirectLightSpacePos[CASCADE_NUM];
};

layout(location = 0) out vec4 outColor;

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
}