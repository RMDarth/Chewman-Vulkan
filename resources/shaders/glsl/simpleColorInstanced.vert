#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
} uniforms;

layout(set = 0, binding = 1) readonly buffer SSBO
{
    mat4 modelList[];
} ssbo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragNormal;
layout (location = 5) out vec3 fragPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 model = ssbo.modelList[gl_InstanceIndex];
    gl_Position = uniforms.projection * uniforms.view * model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(model * vec4(inPosition, 1.0));
    fragNormal = vec3(model * vec4(inNormal, 1.0));
}