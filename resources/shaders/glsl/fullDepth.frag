#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 2, binding = 0) uniform UBO
{
	PointLight pointLight[4];
} ubo;

layout(location = 0) in vec4 fragPos;
layout(location = 1) in flat int fragProjectionNum;

layout(location = 0) out float outColor;

void main()
{
	vec4 LightToVertex = fragPos - ubo.pointLight[fragProjectionNum / 6].position;

    outColor = length(LightToVertex.xyz);
}
