#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D spritesheet;
layout(set = 1, binding = 1) uniform UBO
{
    MaterialInfo materialInfo;
    ivec2 spritesheetSize;
} ubo;

layout(location = 0) in FragData
{
    vec2 fragTexCoord;
    vec4 fragColor;
    float fragLifePercent;
};

layout(location = 0) out vec4 outColor;

vec4 sampleSpritesheet()
{
    int framesNum = ubo.spritesheetSize.x * ubo.spritesheetSize.y - 1;
    int frame = framesNum - int(fragLifePercent * framesNum);
    float frameY = frame / ubo.spritesheetSize.x; // row
    float frameX = frame % ubo.spritesheetSize.x; // column
    float inFramePosX = fragTexCoord.x / ubo.spritesheetSize.x;
    float inFramePosY = fragTexCoord.y / ubo.spritesheetSize.y;

    return texture(spritesheet, vec2(inFramePosX + frameX / ubo.spritesheetSize.x,
                                     inFramePosY + frameY / ubo.spritesheetSize.y));
}

void main()
{
    if (fragLifePercent <= 0)
        discard;
    vec4 result = sampleSpritesheet() * ubo.materialInfo.diffuse;
    outColor = vec4(vec4(result.rgb, result.a) + fragColor);
}
