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

layout(location = 0) in InData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragBinormal;
    vec3 fragTangent;
    vec3 fragPos;
};

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColorBloom;

/// Lava effect

const float NoiseResolution = 4.0; //  0.4
const float Lacunarity = 2.0; // 2.0 // intuitive measure of gappiness / heterogenity or variability
const float Gain = 0.6; // 0.6
const float Ball_rad = 0.45; // 0.45
const float Ball_roll_spd = 0.5; // 0.5
const float Dark_lava_spd = 0.05; // 0.05
const float Dark_island_spd = 0.5; // 0.5


// Let's get random numbers
vec2 random2D(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
    dot(p, vec2(269.5, 183.3))))*43758.5453);
}

float random1D(vec2 p) {
    return fract(sin(dot(p.xy,vec2(12.9898,78.233))) * 43758.5453123);
}


// Add a bit of noise
float noise2D(vec2 _pos) {
    vec2 i = floor(_pos); 		// integer
    vec2 f = fract(_pos); 		// fraction

    // define the corners of a tile
    float a = random1D(i);
    float b = random1D(i + vec2(1.0, 0.0));
    float c = random1D(i + vec2(0.0, 1.0));
    float d = random1D(i + vec2(1.0, 1.0));

    // smooth Interpolation
    vec2 u = smoothstep(0.0, 1.0, f);

    // lerp between the four corners
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// fractal brownian motion
float fbm(vec2 _pos) {
    _pos.y += ubo.time * Ball_roll_spd;
    _pos.x += sin(ubo.time * Ball_roll_spd);
    float ts = ubo.time * Dark_lava_spd;
    float val = 0.0;
    float amp = 0.4;

    // Loop of octaves
    for (int i = 0; i < 4; ++i) // set octave number to 4
    {
        val += amp * noise2D(_pos+ts);
        _pos *= Lacunarity;
        amp *= Gain;
    }
    return val;
}

float voronoiIQ(vec2 _pos) {
    _pos.y += ubo.time * Ball_roll_spd;
    _pos.x += sin(ubo.time * Ball_roll_spd);
    vec2 p = floor(_pos);
    vec2  f = fract(_pos);
    float res = 0.0;
    for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++)
    {
        vec2 b = vec2(i, j);
        vec2 pnt = random2D(p + b);
        pnt = 0.5 + 0.5*sin((ubo.time * Dark_island_spd) + 6.2831*pnt);
        vec2 r = vec2(b) - f + pnt;
        float d = dot(r, r);
        res += exp(-32.0*d); // quickly decaying exponential
    }
    return -(1.0 / 32.0)*log(res);
}

vec3 calculateColor(vec2 texCoord)
{
    float r = 0.0;
    float g = 0.0;
    float b = 0.0;
    vec3 color = vec3(0.0, 0.0, 0.0);

    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = texCoord;
    vec2 pos1 = uv.xy - vec2(0.825, 0.5) ; // center what being drawn
    vec3 pos = vec3(pos1, sqrt(Ball_rad*Ball_rad - pos1.x*pos1.x - pos1.y*pos1.y)/ NoiseResolution);

    float dist = distance(pos.xy, vec2(0.0, 0.0));
    //pos /= vec3(1.0*pos.z, 1.0*pos.z, 0.0);
    pos = pos * 8.0;


    color.rg = vec2(voronoiIQ(pos.xy));
    color.r += 0.25+fbm(pos.xy);

    return color;
}

// parallax
float getHeight(vec2 texCoords)
{
    vec3 color = calculateColor(texCoords);

    return length(vec3(1,1,0) - color);
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

void main() {
    mat3 tanToModel = mat3(fragTangent, fragBinormal, fragNormal);
    mat3 modelToTan = transpose(tanToModel);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 tangentViewDir = normalize(modelToTan * viewDir);
    vec2 parallaxCoord = steepParallaxMapping(fragTexCoord * 0.7, viewDir);

    vec3 color = calculateColor(parallaxCoord);

    // Output to screen
    outColor = vec4(color,1.0);
    outColorBloom = outColor;
}