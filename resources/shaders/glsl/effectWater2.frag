// based on https://www.shadertoy.com/view/lltXD4
#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D mainSampler;
layout(set = 1, binding = 1) uniform sampler2D reflectSampler;
layout(set = 1, binding = 2) uniform UBO
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

// Slow, but more octaves
#define COMPLEX
#define TILED

// Hash function from https://www.shadertoy.com/view/4djSRW
#define HASHSCALE4 vec4(1031, .1030, .0973, .1099)
vec4 hash43(vec3 p)
{
    vec4 p4 = fract(vec4(p.xyzx)  * HASHSCALE4);
    p4 += dot(p4, p4.wzxy+19.19);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

float drip(vec2 uv, vec2 pos, float age, float scale, float cells) {
    vec2 vD = vec2 (uv - pos);
    float fD = sqrt(dot (vD, vD)) * 2.0 * (cells/16.0);
    float fDa = 10.0 * fD;
    float freq = 300.0 * scale;
    return    max (0.0, 1.0 - fDa*fDa)
    * sin ((fD*freq - age*40.0*(scale*2.0-1.0))*hardness);
}

// Based on texture bombing: http://http.developer.nvidia.com/GPUGems/gpugems_ch20.html
float drops(vec2 uv, float cells) {
    float height = 0.0;
    vec2 cell = floor(uv * cells);
    for(int iter=0; iter<iterations; iter++) {
        for(int i = -1; i <= 1; i++) {
            for(int j = -1; j <= 1; j++) {
                vec2 cell_t = cell + vec2(i, j);
                vec2 uv_t = uv;
                #ifdef TILED
                // could be simplified...
                if (cell_t.x<0.0) {
                    cell_t.x += cells;
                    uv_t.x += 1.0;
                } else if (cell_t.x>cells-1.0) {
                    cell_t.x -= cells;
                    uv_t.x -= 1.0;
                }

                if (cell_t.y<0.0) {
                    cell_t.y += cells;
                    uv_t.y += 1.0;
                } else if (cell_t.y>cells-1.0) {
                    cell_t.y -= cells;
                    uv_t.y -= 1.0;
                }
                    #endif
                vec4 rnd_t = hash43(vec3(cell_t, float(iter)));
                vec2 pos_t = (cell_t+rnd_t.xy)/cells;
                float age_t = (ubo.time * speed + rnd_t.z);
                float scale_t = rnd_t.w;
                height += drip(uv_t, pos_t, age_t, scale_t, cells);
            }
        }
    }
    return height;
}

float heightmap(vec2 uv) {
    float height = 0.0;
    #ifdef COMPLEX
    height += drops(uv, 32.0);
    height += drops(uv, 16.0);
    height += drops(uv, 8.0);
    height += drops(uv, 4.0);
    height += drops(uv, 2.0);
    height /= 8.0;
    #else
    height += drops(uv, 8.0);
    height += drops(uv, 4.0);
    height /= 5.0;
    #endif
    return height * intensity;
}

vec2 dudvmap(vec2 uv) {
    const float eps = 0.01;
    vec2 offset = vec2(eps, 0.0);
    return vec2(
        heightmap(uv+offset.xy) - heightmap(uv-offset.xy),
        heightmap(uv+offset.yx) - heightmap(uv-offset.yx)
    );
}

vec3 getColor()
{
    vec2 uv = fragTexCoord;
    #ifdef TILED
    // Tile UVs 2x2 for demonstration
    uv *= 7.0;
    uv = fract(uv);
    #endif

    float height = heightmap(uv);
    vec2 dudv = dudvmap(uv);
    vec3 normal = normalize( vec3(dudv, sqrt(max(1.0-dot(dudv,dudv),0.0))) );

    vec3 color = texture(mainSampler, uv + dudv*0.125).rgb;

    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 lightEffect = CalcDirLight(ubo.dirLight, normal, viewDir, ubo.materialInfo);

    vec3 finalLight = mix(lightEffect, vec3(1,1,1), step(0.5, ubo.dirLight.diffuse.r));
    return color * finalLight;
}

void main() {
    vec3 color = getColor();

    outColor = vec4(color, 0.7);
    outColorBloom = vec4(color * 0, 0.0);
}