#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 blurTextureCoords[11];

layout(location = 0) out vec4 outColor;

const float gausBlur[11] = float[] (0.055037, 0.072806, 0.090506, 0.105726, 0.116061, 0.119726, 0.116061, 0.105726, 0.090506, 0.072806, 0.055037);

void updateColor(inout vec4 color, int position)
{
	vec4 sampled = texture(texSampler, blurTextureCoords[position]);
	color.rgb +=  sampled.rgb * gausBlur[position];
	color.a = max(color.a, sampled.a);
}

void main() {
	outColor = vec4(0.0);
	updateColor(outColor, 0);
	updateColor(outColor, 1);
	updateColor(outColor, 2);
	updateColor(outColor, 3);
	updateColor(outColor, 4);
	updateColor(outColor, 5);
	updateColor(outColor, 6);
	updateColor(outColor, 7);
	updateColor(outColor, 8);
	updateColor(outColor, 9);
	updateColor(outColor, 10);
}
