#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(texSampler, fragTexCoord).rgb;
	float brightness = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
	if (brightness > 0.7)
		outColor = vec4(color, 1.0);
	else
		outColor = vec4(0,0,0,1.0);

	outColor = vec4(color * brightness* brightness, 1.0);
}
