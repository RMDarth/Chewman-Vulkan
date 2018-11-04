#version 450
#extension GL_ARB_separate_shader_objects : enable

const int MAX_SPLITS = 5;
const int VERTEX_NUM = 3; // in triangle

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (set = 1, binding = 0) uniform UBO
{
	mat4 cascadeViewProjection[MAX_SPLITS];
} uniforms;

void main()
{
    for(int cascade = 0; cascade < MAX_SPLITS; cascade++)
    {
        gl_Layer = cascade; // built-in variable that specifies to which face we render.
        for(int i = 0; i < VERTEX_NUM; i++)
        {
            gl_Position = uniforms.cascadeViewProjection[cascade] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}