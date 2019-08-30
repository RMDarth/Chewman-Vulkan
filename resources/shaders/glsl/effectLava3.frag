// Noise animation - Lava
// by nimitz (twitter: @stormoid)
// https://www.shadertoy.com/view/lslXRS
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
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
#define time ubo.time*0.03

float hash21(in vec2 n){ return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453); }
mat2 makem2(in float theta){float c = cos(theta);float s = sin(theta);return mat2(c,-s,s,c);}
float noise( in vec2 x ){return texture(noiseSampler, x*.01).x;}

vec2 gradn(vec2 p)
{
    float ep = 0.075; // 0.09
    float gradx = noise(vec2(p.x+ep,p.y))-noise(vec2(p.x-ep,p.y));
    float grady = noise(vec2(p.x,p.y+ep))-noise(vec2(p.x,p.y-ep));
    return vec2(gradx,grady);
}

float flow(in vec2 p)
{
    float z=2.;
    float rz = 0.;
    vec2 bp = p;
    for (float i= 1.;i < 7.;i++ )
    {
        //primary flow speed
        p += time*.6;

        //secondary flow speed (speed of the perceived flow)
        bp += time*1.9;

        //displacement field (try changing time multiplier)
        vec2 gr = gradn(i*p*.34+time*1.);

        //rotation of the displacement field
        gr*=makem2(time*6.-(0.05*p.x+0.03*p.y)*40.);

        //displace the system
        p += gr*.5;

        //add noise octave
        rz+= (sin(noise(p)*7.)*0.5+0.5)/z;

        //blend factor (blending displaced system with base system)
        //you could call this advection factor (.5 being low, .95 being high)
        p = mix(bp,p,.77);

        //intensity scaling
        z *= 1.4;
        //octave scaling
        p *= 2.;
        bp *= 1.9;
    }
    return rz;
}

vec3 calculateColor(vec2 texCoord)
{
    vec2 p = texCoord - 0.5;
    p *= 45.;
    float rz = flow(p);

    vec3 col = vec3(.6, 0.17, 0.01)/rz;
    col = pow(col, vec3(1.5));

    return col;
}

void main() {
    vec3 color = calculateColor(fragTexCoord);
    float stepY =  1.0 - abs(fragTexCoord.y - 0.5) * 2;
    float stepX =  1.0 - abs(fragTexCoord.x - 0.5) * 2;
    float step = min(stepX, stepY);
    step = step * 15;
    color.r = min(color.r, step);
    color.g = min(color.g, step);
    color.b = min(color.b, step);
    //color.g = smoothstep(0.0, color.g, step);
    outColor = vec4(color,1.0);
    outColorBloom = vec4(color, 0.5);
}