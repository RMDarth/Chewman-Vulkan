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

float waterMix = 0.001;
float waterSpeed = 0.004;
float waveLength = 100;
float waterDir = 0;
float reflection = 0.7;
float textureSize = 5;

vec3 unpackNormal(vec4 color)
{
    return color.xyz * 2 - 1;
}

vec3 getColor()
{
    vec2 scrollMain = fragTexCoord;
    vec2 scrollSec = fragTexCoord;
    vec2 scrollRef1 = fragTexCoord;
    vec2 scrollRef2 = fragTexCoord;
    float waterSpeedX =waterSpeed * 500.0 / textureSize;
    float x1s = sin(waterDir * 0.0174533) * (cos(ubo.time * waterSpeedX ) * waveLength * 0.01); //onda
    float y1s = cos(waterDir * 0.0174533) * (cos(ubo.time * waterSpeedX ) * waveLength * 0.01);
    float refX1 = sin((waterDir + 25) * 0.0174533) * ubo.time * waterSpeedX; //riflesso
    float refY1 = cos((waterDir + 25) * 0.0174533) * ubo.time * waterSpeedX;
    float refX2 = sin((waterDir - 25) * 0.0174533) * ubo.time * waterSpeedX; //riflesso
    float refY2 = cos((waterDir - 25) * 0.0174533) * ubo.time * waterSpeedX;
    scrollMain += vec2(x1s * 0.01, y1s * 0.01 );
    scrollSec += vec2(-x1s * 0.01 * 5, -y1s * 0.01 * 5);
    scrollRef1 += vec2(refX1 * 0.00003, refY1 * 0.00003);
    scrollRef2 += vec2(refX2 * 0.00003, refY2 * 0.00003);
    vec4 diffuse = texture(mainSampler, scrollMain * textureSize) * texture(mainSampler, scrollSec * - waterMix * textureSize);
    //float4 c = tex2D(_MainTex, IN.uv_MainTex) * _Color;
    vec3 normal = unpackNormal(texture (reflectSampler, scrollRef1*textureSize)) + unpackNormal (texture (reflectSampler, scrollRef2*textureSize));
    normal = normalize(normal);

    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 lightEffect = CalcDirLight(ubo.dirLight, normal, viewDir, ubo.materialInfo);

    return lightEffect * 1.5 * diffuse.rgb;
}

void main() {
    vec3 color = getColor();

    outColor = vec4(color, 0.7);
    outColorBloom = vec4(color * 0, 0.0);
}