#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D originalSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 highlight = texture(texSampler, fragTexCoord);
	vec3 color = texture(originalSampler, fragTexCoord).rgb;
	vec3 result = color + highlight.rgb * highlight.a * 10;
	outColor = vec4(result, 1.0);
}
