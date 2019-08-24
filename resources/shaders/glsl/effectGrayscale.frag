#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = texture(texSampler, fragTexCoord).rgb;
	float grayscale = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
	outColor = vec4(grayscale, grayscale, grayscale, 1.0);
}
