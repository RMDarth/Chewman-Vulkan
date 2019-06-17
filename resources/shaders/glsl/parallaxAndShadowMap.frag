#version 450
#extension GL_ARB_separate_shader_objects : enable
const uint MAX_LIGHTS = 3;
const uint CASCADE_NUM = 5;
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D normalTex;
layout(set = 1, binding = 2) uniform sampler2D depthTex;
layout(set = 1, binding = 3) uniform sampler2DArray directShadowTex;
layout(set = 1, binding = 4) uniform samplerCubeArray pointShadowTex;
layout(set = 1, binding = 5) uniform UBO
{
	vec4 cameraPos;
    mat4 invModel;
	DirLight dirLight;
	SpotLight spotLight;
	PointLight pointLight[4];
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
    vec4 fragPointLightSpacePos[MAX_LIGHTS];
    vec4 fragDirectLightSpacePos[CASCADE_NUM];
};

layout(location = 0) out vec4 outColor;

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.1;
    float height = texture(depthTex, texCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale);
    return texCoords + p;
}

vec2 steepParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height_scale = 0.21;
    // number of depth layers
    const float numLayers = 30.0;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaTexCoords = P / numLayers;
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthTex, currentTexCoords).r;


    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthTex, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthTex, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    //return texCoords;
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

// for point
float shadowPointLight(vec4 lightSpacePos, vec3 offset, uint layer)
{
    vec3 fragToLight = fragPos - ubo.pointLight[layer].position.xyz;
    float closestDepth = texture(pointShadowTex, vec4(vec3(fragToLight.x, fragToLight.y, -fragToLight.z) + offset, layer)).r;
    float currentDepth = length(fragToLight);

    return currentDepth < closestDepth + 0.05 ? 1.0 : 0.4;
}

float PCFShadowPointLight(vec4 lightSpacePos, uint layer)
{
	ivec2 texDim = textureSize(pointShadowTex, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int range = 1;

	const vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );
    const int count = 20;
    const float diskRadius = 0.05;

	for(int i = 0; i < count; ++i)
    {
        shadowFactor += shadowPointLight(lightSpacePos, sampleOffsetDirections[i] * diskRadius, layer);
	}
	return shadowFactor / count;
}

// for directional
float PCFShadowSunLight()
{
    ivec2 texDim = textureSize(directShadowTex, 0).xy;
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;
    uint closestLayer = CASCADE_NUM - 1;
    uint layer;
    for (layer = 0; layer < CASCADE_NUM; layer++)
    {
        if (abs(fragDirectLightSpacePos[layer].x) <= 1.0 &&
            abs(fragDirectLightSpacePos[layer].y) <= 1.0 &&
            abs(fragDirectLightSpacePos[layer].z) <= 1.0)
        {
            closestLayer = layer;
            break;
        }
    }

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
            vec2 offset = vec2(dx*x, dy*y);
            vec2 shadowMapCoord = fragDirectLightSpacePos[closestLayer].xy * 0.5 + 0.5;

            // Check if the sample is in the light or in the shadow
            if (fragDirectLightSpacePos[closestLayer].z > texture(directShadowTex, vec3(shadowMapCoord.xy + offset, closestLayer)).x)
            {
                shadowFactor += 0.4;
            } else {
                shadowFactor += 1.0;
            }

            count++;
        }

    }
    return shadowFactor / count;
}

void main()
{
    mat3 modelToTan = mat3(fragTangent, fragBinormal, fragNormal);
    mat3 tanToModel = transpose(modelToTan);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 tangentViewDir = normalize(modelToTan * viewDir);
    vec2 texCoord = steepParallaxMapping(fragTexCoord, tangentViewDir);

    //if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
    //    discard;

    vec3 diffuse = vec3(texture(diffuseTex, texCoord).rgb);
    vec3 normalData = vec3(texture(normalTex, texCoord).rgb);

    //vec3 norm = normalize(fragNormal);
    vec3 normal = modelToTan * (normalData.xyz * 2.0 - 1.0); // to object space

    //normal = normalize(fragNormal);

    vec3 lightEffect = vec3(0);
    float shadow = 0;
    uint count = 0;
    if ((ubo.lightInfo.lightFlags & LI_DirectionalLight) != 0)
    {
        lightEffect += CalcDirLight(ubo.dirLight, normal, viewDir, ubo.materialInfo);
        shadow += PCFShadowSunLight();
        count++;
    }
    for (uint i = 0; i < 4; i++)
    {
        if ((ubo.lightInfo.lightFlags & LI_PointLight[i]) != 0)
        {
            lightEffect += CalcPointLight(ubo.pointLight[i], normal, fragPos, viewDir, ubo.materialInfo);
            shadow += PCFShadowPointLight(fragPointLightSpacePos[i], i);
            count ++;
        }
    }
    if ((ubo.lightInfo.lightFlags & LI_SpotLight) != 0)
        lightEffect += CalcSpotLight(ubo.spotLight, normal, fragPos, viewDir, ubo.materialInfo);

    vec3 result = diffuse * lightEffect * fragColor * (shadow / count);
    //vec3 result = vec3(shadow);
    outColor = vec4(result, 1.0);
}