#version 450
#extension GL_ARB_separate_shader_objects : enable
const uint  MAX_LIGHTS = 3;
const uint  CASCADE_NUM = 5;
layout (set = 0, binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 lightPointViewProjection[MAX_LIGHTS];
	mat4 lightDirectViewProjection[CASCADE_NUM];
	vec4 clipPlane;
} uniforms;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inBinormal;
layout (location = 5) in vec3 inTangent;


layout(location = 0) out OutData
{
    vec3 fragColor;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 fragBinormal;
    vec3 fragTangent;
    vec3 fragPos;
    vec4 fragPointLightSpacePos[MAX_LIGHTS];
    vec4 fragDirectLightSpacePos[CASCADE_NUM];
};

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
    fragNormal = vec3(uniforms.model * vec4(inNormal.xyz, 1.0));
    fragBinormal = vec3(uniforms.model * vec4(inBinormal.xyz, 1.0));
    fragTangent = vec3(uniforms.model * vec4(inTangent.xyz, 1.0));

    float distance = length(camPos.xyz);
    distance = distance - (shadowDistance - transitionDistance);
    distance = distance / transitionDistance;

    for (uint i = 0; i < MAX_LIGHTS; i++)
    {
        //fragPointLightSpacePos[i] =  uniforms.lightPointViewProjection[i] * worldPos;
        //fragPointLightSpacePos[i].xyz = fragPointLightSpacePos[i].xyz / fragPointLightSpacePos[i].w;

        //fragPointLightSpacePos[i].w = clamp(1.0 - distance, 0.0, 1.0);
    }

    for (uint i = 0; i < CASCADE_NUM; i++)
    {
        fragDirectLightSpacePos[i] =  uniforms.lightDirectViewProjection[i] * worldPos;
        fragDirectLightSpacePos[i].xyzw = fragDirectLightSpacePos[i].xyzw / fragDirectLightSpacePos[i].w;

        //fragDirectLightSpacePos[i].w = clamp(1.0 - distance, 0.0, 1.0);
    }
}