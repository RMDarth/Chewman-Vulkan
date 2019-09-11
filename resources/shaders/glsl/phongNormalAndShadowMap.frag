#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D normalTex;
layout(set = 1, binding = 2) uniform sampler2D directShadowTex;
layout(set = 1, binding = 3) uniform UBO
{
	vec4 cameraPos;
    mat4 invModel;
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
    vec3 fragBinormal;
    vec3 fragTangent;
    vec3 fragPos;
    vec4 fragDirectLightSpacePos;
};

layout(location = 0) out vec4 outColor;

#include "lightCalculation.glsl"

void main()
{
    vec3 diffuse = vec3(texture(diffuseTex, fragTexCoord).rgb);
    vec3 normalData = vec3(texture(normalTex, fragTexCoord).rgb);

    //vec3 norm = normalize(fragNormal);
    mat3 rotation = mat3(fragTangent, fragBinormal, fragNormal);
    vec3 normal = rotation * (normalData.xyz * 2.0 - 1.0); // to object space
    mat3 iTWRot = mat3(ubo.invModel[0].xyz, ubo.invModel[1].xyz, ubo.invModel[2].xyz);
    normal = normalize(iTWRot * normal);

    //normal = normalize(fragNormal);

    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

    vec3 lightEffect = calculateLight(normal, viewDir);

    vec3 result = diffuse * lightEffect * fragColor;
    //vec3 result = vec3(shadow);
    outColor = vec4(result, 1.0);
}