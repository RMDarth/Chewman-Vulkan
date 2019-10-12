#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "particles.glsl"

layout (set = 0, binding = 0) uniform UBO
{
    ParticleEmitter emitter;
	mat4 model;
	mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) in vec4 inPositionLife;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inSpeedSize;
layout (location = 3) in vec4 inData;
layout (location = 4) in vec4 inStartData;

layout(location = 0) out OutData
{
    vec2 fragTexCoord;
    vec4 fragColor;
    float fragLifePercent;
};


vec2 rotateVector(float x, float y)
{
    float cosAngle = cos(inData.y);
    float sinAngle = sin(inData.y);
    return vec2(cosAngle*x - sinAngle*y, sinAngle*x + cosAngle*y);
}

void main()
{
    if (inPositionLife.w > 0)
    {
        float halfSize = inSpeedSize.w * 0.5;
        float halfSizeHoriz = halfSize * ubo.emitter.sizeScale;
        vec4 position = ubo.view * ubo.model * vec4(inPositionLife.xyz, 1);
        fragColor = inColor;
        fragLifePercent = inPositionLife.w / inStartData.x;

        int index = gl_VertexIndex % 6;
        if (index == 0)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, halfSizeHoriz), 0.0, 0.0));
            fragTexCoord = vec2(0.0, 1.0);
        }
        else if (index == 1)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, -halfSizeHoriz), 0.0, 0.0 ));
            fragTexCoord = vec2(0.0, 0.0);
        }
        else if (index == 2)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, halfSizeHoriz), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 1.0);
        }
        else if (index == 3)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, halfSizeHoriz), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 1.0);
        }
        else if (index == 4)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, -halfSizeHoriz), 0.0, 0.0 ));
            fragTexCoord = vec2(0.0, 0.0);
        }
        else
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, -halfSizeHoriz), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 0.0);
        }
    } else {
        fragColor = vec4(0);
        fragLifePercent = 0.0;
        fragTexCoord = vec2(0, 0);
        gl_Position = vec4(0);
    }
}