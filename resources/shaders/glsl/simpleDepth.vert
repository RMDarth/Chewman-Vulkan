#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 viewProjection;
} matrices;

layout (location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	gl_Position = matrices.viewProjection * matrices.model * vec4(inPosition, 1.0);
}
