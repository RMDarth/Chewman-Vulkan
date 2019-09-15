// Based on "Burning Texture Fade" by Krzysztof Kondrak @k_kondrak
// https://www.shadertoy.com/view/XsVfWz
#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D directShadowTex;
layout(set = 1, binding = 2) uniform UBO
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

#include "lightCalculation.glsl"

float r(in vec2 p)
{
    return fract(cos(p.x*42.98 + p.y*43.23) * 1127.53);
}

// using noise functions from: https://www.shadertoy.com/view/XtXXD8
float n(in vec2 p)
{
    vec2 fn = floor(p);
    vec2 sn = smoothstep(vec2(0), vec2(1), fract(p));

    float h1 = mix(r(fn), r(fn + vec2(1,0)), sn.x);
    float h2 = mix(r(fn + vec2(0,1)), r(fn + vec2(1)), sn.x);
    return mix(h1 ,h2, sn.y);
}

float noise(in vec2 p)
{
    return n(p/32.) * 0.58 +
    n(p/16.) * 0.2  +
    n(p/8.)  * 0.1  +
    n(p/4.)  * 0.05 +
    n(p/2.)  * 0.02 +
    n(p)     * 0.0125;
}

void main()
{
    vec3 diffuse = vec3(texture(diffuseTex, fragTexCoord).rgb);
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 lightEffect = calculateLight(normal, viewDir);
    vec3 color = diffuse * lightEffect * fragColor;

    float t = mod(ubo.time*0.25, 1.2);
    // fade to black
    vec4 result = mix(vec4(color, 1.0), vec4(0,0,0,0), smoothstep(t + .1, t - .1, noise(fragTexCoord * 100.0)));
    // burning on the edges (when c.a < .1)
    result.rgb = clamp(result.rgb + step(result.a, .1) * 1.6 * noise(2000. * fragTexCoord) * vec3(1.2,.5,.0), 0., 1.);
    result.a = mix(0.5, 1.0, smoothstep(0.0, 0.1, result.a));

    outColor = result;
}