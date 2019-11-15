#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D noiseSampler;
layout(set = 1, binding = 1) uniform UBO
{
    vec4 cameraPos;
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

/// Lava effect

vec3 grad(vec3 p)
{
    const float texture_width = 256.0;
    vec4 v = texture(noiseSampler, vec2((p.x+p.z) / texture_width, (p.y-p.z) / texture_width), 0);
    return normalize(v.xyz*2.0 - vec3(1.0));
}

/* S-shaped curve for 0 <= t <= 1 */
float fade(float t)
{
    return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
}


/* 3D noise */
float noise(vec3 p) {
    /* Calculate lattice points. */
    vec3 p0 = floor(p);
    vec3 p1 = p0 + vec3(1.0, 0.0, 0.0);
    vec3 p2 = p0 + vec3(0.0, 1.0, 0.0);
    vec3 p3 = p0 + vec3(1.0, 1.0, 0.0);
    vec3 p4 = p0 + vec3(0.0, 0.0, 1.0);
    vec3 p5 = p4 + vec3(1.0, 0.0, 0.0);
    vec3 p6 = p4 + vec3(0.0, 1.0, 0.0);
    vec3 p7 = p4 + vec3(1.0, 1.0, 0.0);

    /* Look up gradients at lattice points. */
    vec3 g0 = grad(p0);
    vec3 g1 = grad(p1);
    vec3 g2 = grad(p2);
    vec3 g3 = grad(p3);
    vec3 g4 = grad(p4);
    vec3 g5 = grad(p5);
    vec3 g6 = grad(p6);
    vec3 g7 = grad(p7);

    float t0 = p.x - p0.x;
    float fade_t0 = fade(t0); /* Used for interpolation in horizontal direction */

    float t1 = p.y - p0.y;
    float fade_t1 = fade(t1); /* Used for interpolation in vertical direction. */

    float t2 = p.z - p0.z;
    float fade_t2 = fade(t2);

    /* Calculate dot products and interpolate.*/
    float p0p1 = (1.0 - fade_t0) * dot(g0, (p - p0)) + fade_t0 * dot(g1, (p - p1)); /* between upper two lattice points */
    float p2p3 = (1.0 - fade_t0) * dot(g2, (p - p2)) + fade_t0 * dot(g3, (p - p3)); /* between lower two lattice points */

    float p4p5 = (1.0 - fade_t0) * dot(g4, (p - p4)) + fade_t0 * dot(g5, (p - p5)); /* between upper two lattice points */
    float p6p7 = (1.0 - fade_t0) * dot(g6, (p - p6)) + fade_t0 * dot(g7, (p - p7)); /* between lower two lattice points */

    float y1 = (1.0 - fade_t1) * p0p1 + fade_t1 * p2p3;
    float y2 = (1.0 - fade_t1) * p4p5 + fade_t1 * p6p7;

    /* Calculate final result */
    return (1.0 - fade_t2) * y1 + fade_t2 * y2;
}

// parallax
float getHeight(vec2 texCoords)
{
    vec2 texCoordsScaled = vec2(texCoords.x * 1000, texCoords.y * 1000);
    float n =
        noise(vec3(texCoordsScaled, ubo.time * 30.0)/128.0)/ 1.0 +
        noise(vec3(texCoordsScaled, ubo.time * 30.0)/64.0) / 2.0 +
        noise(vec3(texCoordsScaled, ubo.time * 64.0)/32.0) / 16.0;
    if (n > 0.001) n = n * 3.0;

    return length(vec2(0,0) - mix(vec2(1.0, 0.6), vec2(0.54, 0.0), n*1.2 + 0.4));
}

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.3;
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

void main() {
    mat3 tanToModel = mat3(fragTangent, fragBinormal, fragNormal);
    mat3 modelToTan = transpose(tanToModel);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 tangentViewDir = normalize(tanToModel * viewDir);
    vec2 parallaxCoord = parallaxMapping(fragTexCoord + vec2(ubo.time * 0.1, 0), viewDir);
    vec2 texCoord = vec2(parallaxCoord.x * 1000, parallaxCoord.y * 1000);
    float n =
        noise(vec3(texCoord, ubo.time * 30.0)/128.0)/ 1.0 +
        noise(vec3(texCoord, ubo.time * 30.0)/64.0) / 2.0 +
        noise(vec3(texCoord, ubo.time * 64.0)/32.0) / 16.0;
    if (n > 0.001) n = n * 3.0;

    outColor = vec4(mix(vec3(1.0, 0.6, 0.0), vec3(0.54, 0.0, 0.0), n*1.2 + 0.4) , 1.0);
}