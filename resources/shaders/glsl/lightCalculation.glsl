// Copyright (c) 2018-2019, Igor Barinov
// This file should be included after UBO declaration

/////// HELPER FUNCTIONS ////////////

// for directional
float PCFShadowSunLight()
{
    ivec2 texDim = textureSize(directShadowTex, 0).xy;
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 2;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
            vec2 offset = vec2(dx*x, dy*y);
            vec2 shadowMapCoord = fragDirectLightSpacePos.xy * 0.5 + 0.5;

            // Check if the sample is in the light or in the shadow
            if (fragDirectLightSpacePos.z > texture(directShadowTex, vec2(shadowMapCoord.xy + offset)).x)
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

float SimpleShadowSunLight()
{
    // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
    vec2 shadowMapCoord = fragDirectLightSpacePos.xy * 0.5 + 0.5;

    // Check if the sample is in the light or in the shadow
    if (fragDirectLightSpacePos.z > texture(directShadowTex, shadowMapCoord.xy).x)
        return 0.4; // In the shadow

    // In the light
    return 1.0;
}

/////// Light and shadow calculation ////////////
vec3 calculateLight(vec3 normal, vec3 viewDir)
{
    vec3 lightEffect = vec3(0);
    float shadow = 0;
    uint count = 0;

    if ((ubo.lightInfo.lightFlags & LI_DirectionalLight) != 0)
    {
        vec3 curLight = CalcDirLight(ubo.dirLight, normal, viewDir, ubo.materialInfo);
        shadow = SimpleShadowSunLight();
        lightEffect += curLight * (shadow);
        count++;
    }

    uint shadowCount = 0;

    //for (uint i = 0; i < ubo.lightInfo.lightLineNum; i++)
    //{
    //    lightEffect += CalcLineLight(ubo.lineLight[i], normal, fragPos, viewDir, ubo.materialInfo);
    //}
//
    //for (uint i = 0; i < ubo.lightInfo.lightPointsNum; i++)
    //{
    //    lightEffect += CalcPointLight(ubo.pointLight[i], normal, fragPos, viewDir, ubo.materialInfo);
    //}

    //if ((ubo.lightInfo.lightFlags & LI_SpotLight) != 0)
    //    lightEffect += CalcSpotLight(ubo.spotLight, normal, fragPos, viewDir, ubo.materialInfo);

    return lightEffect;
}