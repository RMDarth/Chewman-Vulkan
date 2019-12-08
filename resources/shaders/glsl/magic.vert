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

float getRandomValue(float low, float high, float specialData)
{
    // 0...1
    float randomValue = random(specialData);

    return low + (high - low) * randomValue;
}

void main() {
    vec3 color = ubo.configuration[1].rgb;

    int polygonId = gl_VertexIndex / 6;
    int vertexId = gl_VertexIndex % 6;

    float radius = ubo.configuration[0].z;
    float maxHeight = ubo.configuration[1].w;
    float polygonRandom = random(polygonId);
    float xPos = getRandomValue(-radius, radius, 1 + polygonRandom);
    float yPos = getRandomValue(-radius, radius, 2 + polygonRandom);

    float vertPos = -polygonRandom * 9 + ubo.time * ubo.configuration[0].y;
    vertPos = mod(vertPos, maxHeight);
    vec4 position =  vec4(xPos, vertPos, yPos, 1);
    float alpha = maxHeight - vertPos;
    float halfSize = ubo.configuration[0].w * alpha * 0.3;
    float sizeScale = ubo.configuration[0].x;
    float halfSizeHoriz = halfSize * sizeScale;

    gl_Position = ubo.projection * ubo.view * ubo.model * (position + vec4(positions[vertexId].x * halfSize, positions[vertexId].y * halfSizeHoriz, 0.0, 0.0));
    fragTexCoord = texCoord[vertexId];

    fragColor = vec4(color, alpha);
    fragLifePercent = alpha;
}