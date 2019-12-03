// based on https://www.shadertoy.com/view/4tdGWX by 4rknova
#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform UBO
{
    vec4 cameraPos;
    DirLight dirLight;
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
layout(location = 1) out vec4 outColorBloom;

const float hardness = 0.1;
const int iterations = 4;
const float speed = 1.5;
const float intensity = 1.0;

vec3 getColor()
{
    float time = ubo.time * 0.2;
    vec2 c = fragTexCoord * 50. + time * .3;

    float k = .1 + cos(c.y + sin(.148 - time)) + 2.4 * time;
    float w = .9 + sin(c.x + cos(.628 + time)) - 0.7 * time;
    float d = length(c);
    float s = 7. * cos(d+w) * sin(k+w);

    float colS = .6 + .5 * cos(s + 1.0 + c.x+c.y);
    vec3 col = vec3(colS * 0.5, colS, colS * 0.5);

    col *= vec3(.7, 1, .4)
        *  pow(max(normalize(vec3(length(dFdx(col)), length(dFdy(col)), 0.001)).z, 0.), 2.)
        + .75;
    return col;
}

void main() {
    float ambient = mix(0.8, 1.0, step(0.5, ubo.dirLight.diffuse.r));
    vec3 color = getColor() * ambient;

    outColor = vec4(color, 1.0);
    outColorBloom = vec4(color, 0.08);
}