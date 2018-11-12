#version 450
#extension GL_ARB_separate_shader_objects : enable

const uint MAX_LIGHTS = 3;
const uint MAX_MATRICES = MAX_LIGHTS * 6;
const uint VERTEX_NUM = 3; // in triangle

layout (triangles) in;
layout (triangle_strip, max_vertices = 54) out;

layout (set = 1, binding = 0) uniform UBO
{
    ivec4 matrixCount;
	mat4 ViewProjectionMatrices[MAX_MATRICES];
} uniforms;

layout(location = 0) out vec4 fragPos;
layout(location = 1) out flat int fragProjectionNum;

void main()
{
    for(int matrixId = 0; matrixId < uniforms.matrixCount.x; matrixId++)
    {
        gl_Layer = matrixId; // built-in variable that specifies to which face we render.
        fragProjectionNum = matrixId;
        for(int i = 0; i < VERTEX_NUM; i++)
        {
            fragPos = gl_in[i].gl_Position;
            gl_Position = uniforms.ViewProjectionMatrices[matrixId] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}