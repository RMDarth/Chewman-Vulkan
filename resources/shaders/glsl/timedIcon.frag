#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    float timePassed;
} ubo;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 image = texture(texSampler, fragTexCoord);

    vec2 distV = fragTexCoord - 0.5;
    float dist = dot(distV, distV);
    float angle = (1 + atan(distV.x, distV.y) / 3.1415) * 0.5; // from 1.0 to 0.0

    outColor = image + smoothstep(0.18, 0.2, dist) * smoothstep(0.25, 0.23, dist) * step(ubo.timePassed, angle) * vec4(ubo.timePassed,1.0 - ubo.timePassed, 0,1);
}
