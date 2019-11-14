#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrices;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inBinormal;
layout (location = 5) in vec3 inTangent;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragBinormal;
layout (location = 4) out vec3 fragTangent;
layout (location = 5) out vec3 fragPos;


out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = matrices.projection * matrices.view * matrices.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(matrices.model * vec4(inPosition, 1.0));
    fragNormal = vec3(matrices.model * vec4(inNormal, 1.0));
    fragBinormal = vec3(matrices.model * vec4(inBinormal.xyz, 1.0));
    fragTangent = vec3(matrices.model * vec4(inTangent.xyz, 1.0));
}