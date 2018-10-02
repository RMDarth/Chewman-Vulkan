#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 lightViewProjection;
	vec4 clipPlane;
} uniforms;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec4 fragLightSpacePos;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[];
};

void main() {
    vec4 worldPos = uniforms.model * vec4(inPosition, 1.0);

    gl_Position = uniforms.projection * uniforms.view * worldPos;
    gl_ClipDistance[0] = dot(worldPos, uniforms.clipPlane);
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    fragPos = vec3(worldPos);
    fragNormal = vec3(uniforms.model * vec4(inNormal, 1.0)); // transpose(inverse(uniforms.model)) not working for assimp

    fragLightSpacePos =  uniforms.lightViewProjection * worldPos;
}