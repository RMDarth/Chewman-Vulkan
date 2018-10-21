#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2DArray shadowTex;
layout(set = 1, binding = 2) uniform UBO
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
layout(location = 4) in vec4 fragLightSpacePos;

layout(location = 0) out vec4 outColor;


// for directional
float computeShadowFactor(vec4 lightSpacePos, vec2 offset, uint layer)
{
   // Convert light space position to NDC (normalized device coordinates)
   //vec3 lightSpaceReal = lightSpacePos.xyz /= lightSpacePos.w;

   // If the fragment is outside the light's projection then it is outside
   // the light's influence, which means it is in the shadow (notice that
   // such sample would be outside the shadow map image)
   // TODO: Use something like "isFiniteShadow" uniform for this check
   /*if (abs(lightSpacePos.x) > 1.0 ||
       abs(lightSpacePos.y) > 1.0 ||
       abs(lightSpacePos.z) > 1.0)
      return lightSpacePos.w;*/


   // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
   vec2 shadowMapCoord = lightSpacePos.xy * 0.5 + 0.5;

   // Check if the sample is in the light or in the shadow
    if (lightSpacePos.z > texture(shadowTex, vec3(shadowMapCoord.xy + offset, layer)).x)
    {
         return 1 - (lightSpacePos.w * 0.6); // In the shadow
    }

   // In the light

   return 1.0;
}

float filterPCF(vec4 lightSpacePos, uint layer)
{
	ivec2 texDim = textureSize(shadowTex, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += computeShadowFactor(lightSpacePos, vec2(dx*x, dy*y), layer);
			count++;
		}

	}
	return shadowFactor / count;
}

void main() {
    vec3 diffuse = vec3(texture(diffuseTex, fragTexCoord).rgb);

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

    vec3 result = diffuse * lightEffect * fragColor * filterPCF(fragLightSpacePos, 0);
    outColor = vec4(result, 1.0);
}