#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
} ubo;

layout (location = 0) in vec4 inPositionLife;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec4 inSpeedSize;
layout (location = 3) in vec4 inData;
layout (location = 4) in vec4 inStartData;

layout(location = 0) out OutData
{
    vec4 geomColor;
    float geomLife;
    float geomStartLife;
    float geomSize;
    float geomRotation;
};

void main()
{
    gl_Position = ubo.view * ubo.model * vec4(inPositionLife.xyz, 1);
    geomLife = inPositionLife.w;
    geomStartLife = inStartData.x;
    geomColor = inColor;
    geomSize = inSpeedSize.w;
    geomRotation = inData.y;
}