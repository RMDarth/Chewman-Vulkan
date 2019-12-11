#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    vec4 info;
} ubo;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec4 sampleSpritesheet()
{
    int framesNum = int(ubo.info.x * ubo.info.y - 1);
    int frame = int(ubo.info.z * framesNum);
    int frameY = frame / int(ubo.info.x); // row
    int frameX = frame % int(ubo.info.x); // column
    float inFramePosX = fragTexCoord.x / ubo.info.x;
    float inFramePosY = fragTexCoord.y / ubo.info.y;

    return texture(texSampler, vec2(inFramePosX + frameX / ubo.info.x,
                                    inFramePosY + frameY / ubo.info.y));
}

void main()
{
    outColor = sampleSpritesheet();
}
