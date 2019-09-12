#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D noiseSampler;
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
    float time;
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

/// Lava effect
#define time ubo.time*0.13

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
        p += time*.6;
        bp += time*1.9;
        vec2 gr = gradn(i*p*.34+time*1.);
        gr*=makem2(time*6.-(0.05*p.x+0.03*p.y)*40.);
        p += gr*.5;
        rz+= (sin(noise(p)*7.)*0.5+0.5)/z;
        p = mix(bp,p,.77);
        z *= 1.4;
        p *= 2.;
        bp *= 1.9;
    }
    return rz;
}
vec3 calculateLavaColor(vec2 texCoord)
{
    vec2 p = texCoord - 0.5;
    p *= 25.;
    float rz = flow(p);
    vec3 col = vec3(.6, 0.17, 0.01)/rz;
    col = pow(col, vec3(1.5));
    return col;
}

void main()
{
    vec3 diffuse = texture(diffuseTex, fragTexCoord).rgb;

    if (diffuse.r > 0.9 && diffuse.b < 0.4)
    {
        outColor = vec4(calculateLavaColor(fragTexCoord), 1.0);

        float brightness = 0.2126 * outColor.r + 0.7152 * outColor.g + 0.0722 * outColor.b;
        brightness = brightness * step(0.7, brightness);
        outColorBloom = vec4(outColor.rgb * brightness * brightness, 0.3);
    } else {
        vec3 normal = normalize(fragNormal);
        vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

        vec3 lightEffect = calculateLight(normal, viewDir);

        vec3 result = diffuse * lightEffect * fragColor;
        //vec3 result = vec3(shadow);
        outColor = vec4(result, 1.0);
        outColorBloom = vec4(0);
    }
}