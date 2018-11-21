#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
} ubo;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;

layout(location = 0) out vec3 geomColor;

void main()
{
    gl_Position = ubo.view * ubo.model * inPosition;
    geomColor = inColor.rgb;
}