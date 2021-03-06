#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "staticrandom.glsl"

layout (set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 configuration;
    float time;
} ubo;

layout(location = 0) out FragData
{
    vec2 fragTexCoord;
    vec4 fragColor;
    float fragLifePercent;
};

const vec2 positions[6] = vec2[](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
);

const vec2 texCoord[6] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
);

vec2 rotateVector(float x, float y, float angle)
{
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    return vec2(cosAngle*x - sinAngle*y, sinAngle*x + cosAngle*y);
    //return vec2(x, y);
}

void main() {
    float percent = ubo.configuration[2].x;
    float totalVertex = int(ubo.configuration[2].y);

    int polygonId = gl_VertexIndex / 6;
    float limit = (totalVertex * percent) / 6;

    if (polygonId > limit)
    {
        gl_Position = vec4(0);
        fragColor = vec4(0);
        fragTexCoord = vec2(0);
        fragLifePercent = 0;
    }
    else
    {
        int vertexId = gl_VertexIndex % 6;

        vec3 startPos = ubo.configuration[0].xyz;
        vec3 direction = ubo.configuration[1].xyz;

        float totalLength = ubo.configuration[2].z;
        float alpha = ubo.configuration[2].a;
        float currentLength = totalLength * percent;

        float currentPercent = float(polygonId) / limit;
        vec4 position = ubo.view * ubo.model * vec4(startPos + direction * currentLength * currentPercent, 1);
        float polyRandom = random(int(limit + 0.001 - polygonId));
        float minus = 1.0 - 2.0 * step(0.5, polyRandom);
        float angle = 6.3 * polyRandom + ubo.time * 3 * polyRandom * minus;
        float halfSize = 0.4 * polyRandom + 0.2;
        halfSize *= alpha;

        gl_Position = ubo.projection * (position + vec4(rotateVector(positions[vertexId].x * halfSize, positions[vertexId].y * halfSize, angle), 0.0, 0.0));
        fragTexCoord = texCoord[vertexId];

        fragColor = vec4(1,1,1,alpha);
        fragLifePercent = (1.0 - currentPercent) * alpha;
    }
}