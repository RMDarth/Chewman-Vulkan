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
#define PI 3.14159265358979323846
#define HALFPI PI * 0.5

float rand(vec2 c){
    return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 diffuse = vec3(texture(diffuseTex, fragTexCoord).rgb);
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 lightEffect = calculateLight(normal, viewDir);
    vec3 color = diffuse * lightEffect * fragColor;

    //Paremeters
    float p = 0.5; //Distance
    float a = 0.2 * PI; //Angle
    float v = 2.4; //Velocity
    float s = smoothstep(-HALFPI, HALFPI, sin(ubo.time * v));
    float r = rand(fragTexCoord);
    vec2 f = vec2(sin(a) * s, cos(a) * s);
    vec2 uv = fragTexCoord;
    uv.x -= r * f.x * p * s;
    uv.y -= r * f.y * p * s;
    vec4 c = vec4(color, 1.0);
    vec2 n = normalize(uv.xy - f);
    float d = dot(f, n);
    //Todo: Smoothstep?
    vec4 empty = vec4(0.0,0.0,0.0,0.0);
    if (d < 0.0) {
        c = empty; //c.a = 0.0;
    }
    c = mix(c, empty, s); //c.a = s,

    outColor = c;
}