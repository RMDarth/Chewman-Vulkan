#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D normalTex;
layout(set = 1, binding = 2) uniform sampler2D depthTex;
layout(set = 1, binding = 3) uniform sampler2D directShadowTex;
layout(set = 1, binding = 4) uniform UBO
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

float getHeight(vec2 texCoords)
{
    return texture(depthTex, texCoords).r - 0.5;
}

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.2;
    float height = getHeight(texCoords);
    vec2 p = viewDir.xy * (height * heightScale);
    return texCoords - p;
}

vec2 steepParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.1;
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = getHeight(currentTexCoords);

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = getHeight(currentTexCoords);
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = getHeight(prevTexCoords) - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    mat3 tanToModel = mat3(fragTangent, fragBinormal, fragNormal);
    mat3 modelToTan = transpose(tanToModel);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 tangentViewDir = normalize(modelToTan * viewDir);
    vec2 texCoord = steepParallaxMapping(fragTexCoord, tangentViewDir);

    //if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
    //    discard;

    vec3 diffuse = vec3(texture(diffuseTex, texCoord).rgb);
    vec3 normalData = vec3(texture(normalTex, texCoord).rgb);

    //vec3 norm = normalize(fragNormal);
    vec3 normal = tanToModel * (normalData.xyz * 2.0 - 1.0); // to object space

    //normal = normalize(fragNormal);

    vec3 lightEffect = calculateLight(normal, viewDir);

    vec3 result = diffuse * lightEffect * fragColor;
    //vec3 result = vec3(lightEffect);
    outColor = vec4(result, 1.0);
}