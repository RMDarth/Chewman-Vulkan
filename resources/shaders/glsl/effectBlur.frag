#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 blurTextureCoords[11];

layout(location = 0) out vec4 outColor;

const float gausBlur[11] = float[] (0.055037, 0.072806, 0.090506, 0.105726, 0.116061, 0.119726, 0.116061, 0.105726, 0.090506, 0.072806, 0.055037);

void updateColor(inout vec4 color, vec2 texCoord, float gausBlur)
{
	vec4 sampled = texture(texSampler, texCoord);
	color.rgb +=  sampled.rgb * gausBlur;
	color.a = max(color.a, sampled.a);
}

void main() {
	outColor = vec4(0.0);
	updateColor(outColor, blurTextureCoords[0], gausBlur[0]);
	updateColor(outColor, blurTextureCoords[1], gausBlur[1]);
	updateColor(outColor, blurTextureCoords[2], gausBlur[2]);
	updateColor(outColor, blurTextureCoords[3], gausBlur[3]);
	updateColor(outColor, blurTextureCoords[4], gausBlur[4]);
	updateColor(outColor, blurTextureCoords[5], gausBlur[5]);
	updateColor(outColor, blurTextureCoords[6], gausBlur[6]);
	updateColor(outColor, blurTextureCoords[7], gausBlur[7]);
	updateColor(outColor, blurTextureCoords[8], gausBlur[8]);
	updateColor(outColor, blurTextureCoords[9], gausBlur[9]);
	updateColor(outColor, blurTextureCoords[10], gausBlur[10]);
}
