#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "text.glsl"

layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 viewProjection;
	TextInfo textInfo;
} ubo;

layout(location = 0) out vec2 fragTexCoord;

vec2 positions[6] = vec2[](
	vec2(-1.0, 1.0),
	vec2(-1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0)
);

vec2 texCoord[6] = vec2[](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
);

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	fragTexCoord = texCoord[gl_VertexIndex];
}