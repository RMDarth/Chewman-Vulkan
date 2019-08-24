#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 blurTextureCoords[11];

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.0);
	outColor += texture(texSampler, blurTextureCoords[0])  * 0.066414;
	outColor += texture(texSampler, blurTextureCoords[1])  * 0.079465;
	outColor += texture(texSampler, blurTextureCoords[2])  * 0.091364;
	outColor += texture(texSampler, blurTextureCoords[3])  * 0.100939;
	outColor += texture(texSampler, blurTextureCoords[4])  * 0.107159;
	outColor += texture(texSampler, blurTextureCoords[5])  * 0.109317;
	outColor += texture(texSampler, blurTextureCoords[6])  * 0.107159;
	outColor += texture(texSampler, blurTextureCoords[7])  * 0.100939;
	outColor += texture(texSampler, blurTextureCoords[8])  * 0.091364;
	outColor += texture(texSampler, blurTextureCoords[9])  * 0.079465;
	outColor += texture(texSampler, blurTextureCoords[10]) * 0.066414;
}
