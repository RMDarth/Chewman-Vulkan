#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2, binding = 0) uniform sampler2D spritesheet;
layout(set = 2, binding = 1) uniform UBO
{
    ivec2 spritesheetSize;
} ubo;

layout(location = 0) in FragData
{
    vec2 fragTexCoord;
    vec3 fragColor;
    float fragLifePercent;
};

layout(location = 0) out vec4 outColor;

vec4 sampleSpritesheet()
{
    int framesNum = ubo.spritesheetSize.x * ubo.spritesheetSize.y;
    int frame = framesNum - int(fragLifePercent * framesNum);
    float frameY = frame / ubo.spritesheetSize.x; // row
    float frameX = frame % ubo.spritesheetSize.x; // column
    float inFramePosX = fragTexCoord.x / ubo.spritesheetSize.x;
    float inFramePosY = fragTexCoord.y / ubo.spritesheetSize.y;

    //return vec4(vec2(inFramePosX + frameX / ubo.spritesheetSize.x,inFramePosY + frameY / ubo.spritesheetSize.y), 0.0, 1.0);
    return texture(spritesheet, vec2(inFramePosX + frameX / ubo.spritesheetSize.x,
                                     inFramePosY + frameY / ubo.spritesheetSize.y));
}

void main()
{
    outColor = vec4(sampleSpritesheet());
}
