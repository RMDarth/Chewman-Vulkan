#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "overlay.glsl"

layout (std140, set = 0, binding = 0) uniform UBO
{
	OverlayInfo overlayInfo;
	ivec4 imageSize;
} ubo;

layout(location = 0) out vec2 fragTexCoord;

vec2 texCoord[6] = vec2[](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
);

void main() {
	float left = (float(ubo.overlayInfo.x) / ubo.imageSize.x) * 2.0 - 1.0;
	float right = (float(ubo.overlayInfo.x + ubo.overlayInfo.width) / ubo.imageSize.x) * 2.0 - 1.0;
	float top = (float(ubo.overlayInfo.y) / ubo.imageSize.y) * 2.0 - 1.0;
	float bottom = (float(ubo.overlayInfo.y + ubo.overlayInfo.height) / ubo.imageSize.y) * 2.0 - 1.0;

	vec2 positions[6] = vec2[](
		vec2(left,  bottom),
		vec2(left,  top),
		vec2(right, top),
		vec2(right, top),
		vec2(right, bottom),
		vec2(left,  bottom)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	fragTexCoord = texCoord[gl_VertexIndex];
}