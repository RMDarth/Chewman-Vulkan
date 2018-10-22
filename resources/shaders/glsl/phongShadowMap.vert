#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 6
layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 lightViewProjection[MAX_LIGHTS];
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

const float shadowDistance = 20.0;
const float transitionDistance = 2.0;

void main() {
    vec4 worldPos = uniforms.model * vec4(inPosition, 1.0);
    vec4 camPos = uniforms.view * worldPos;

    gl_Position = uniforms.projection * camPos;
    gl_ClipDistance[0] = dot(worldPos, uniforms.clipPlane);
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    fragPos = vec3(worldPos);
    fragNormal = vec3(transpose(inverse(uniforms.model)) * vec4(inNormal.xyz, 1.0)); // transpose(inverse(uniforms.model)) not working for assimp

    fragLightSpacePos =  uniforms.lightViewProjection[0] * worldPos;
    //fragLightSpacePos.xy = fragLightSpacePos.xy / fragLightSpacePos.w;

    //toCameraVector = (inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - worldPosition.xyz;

    float distance = length(camPos.xyz);
    distance = distance - (shadowDistance - transitionDistance);
    distance = distance / transitionDistance;

    fragLightSpacePos.w = clamp(1.0 - distance, 0.0, 1.0);
}