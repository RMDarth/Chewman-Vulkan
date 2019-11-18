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

vec2 rotateVector(float x, float y, float angle)
{
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    return vec2(cosAngle*x - sinAngle*y, sinAngle*x + cosAngle*y);
    //return vec2(x, y);
}

float getRandomValue(float low, float high, float specialData)
{
    // 0...1
    float randomValue = random(specialData);

    return low + (high - low) * randomValue;
}


void main() {
    float percent = ubo.configuration[0].w;
    float totalVertex = int(ubo.configuration[1].y);

    int polygonId = gl_VertexIndex / 6;
    float limit = (totalVertex * percent) / 6;

    if (polygonId * 0.5 > limit)
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

        float radius = 8.5;
        float halfSize = 2.0;
        float polygonRandom = random(polygonId);
        float xPos = getRandomValue(-radius, radius, 1 + polygonRandom);
        float yPos = getRandomValue(-radius, radius, 2 + polygonRandom);
        float angle = getRandomValue(-1.5, 1.5, 3 + polygonRandom);
        vec4 position = ubo.view * ubo.model * vec4(xPos, -polygonRandom + percent * 0.8, yPos, 1);

        if (vertexId == 0)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, halfSize, angle), 0.0, 0.0));
            fragTexCoord = vec2(0.0, 1.0);
        }
        else if (vertexId == 1)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, -halfSize, angle), 0.0, 0.0 ));
            fragTexCoord = vec2(0.0, 0.0);
        }
        else if (vertexId == 2)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, halfSize, angle), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 1.0);
        }
        else if (vertexId == 3)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, halfSize, angle), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 1.0);
        }
        else if (vertexId == 4)
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(-halfSize, -halfSize, angle), 0.0, 0.0 ));
            fragTexCoord = vec2(0.0, 0.0);
        }
        else
        {
            gl_Position = ubo.projection * (position + vec4(rotateVector(halfSize, -halfSize, angle), 0.0, 0.0 ));
            fragTexCoord = vec2(1.0, 0.0);
        }
        fragColor = vec4(1,1,1,1);
        fragLifePercent = clamp(1.0 - (-polygonRandom + percent * 2.0), 0.0, 1.0);
    }
}