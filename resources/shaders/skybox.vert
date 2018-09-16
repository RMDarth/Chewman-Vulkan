#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
	mat4 projection;
} matrices;

layout(location = 0) in vec3 inPosition;

layout (location = 0) out vec3 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
	fragTexCoord = inPosition;
	fragTexCoord.x *= -1.0;
	//fragTexCoord.y = 1.0 - fragTexCoord.y;
	vec4 pos = matrices.projection * mat4(mat3(matrices.view)) *  vec4(inPosition.xyz, 1.0);
	gl_Position = pos.xyzw;
}