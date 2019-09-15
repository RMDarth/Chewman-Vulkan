#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "particles.glsl"

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in InData
{
    vec4 geomColor;
    float geomLife;
    float geomStartLife;
    float geomSize;
    float geomRotation;
} geomData[];

layout(set = 1, binding = 0) uniform UBO
{
    mat4 projection;
    mat4 model;
    mat4 view;
    ParticleEmitter emitter;
} ubo;

layout(location = 0) out FragData
{
    vec2 fragTexCoord;
    vec4 fragColor;
    float fragLifePercent;
};

float cosAngle = cos(geomData[0].geomRotation);
float sinAngle = sin(geomData[0].geomRotation);
vec2 rotateVector(float x, float y)
{
    return vec2(cosAngle*x - sinAngle*y, sinAngle*x + cosAngle*y);
}

void main()
{
    if (geomData[0].geomLife > 0)
    {
        float halfSize = geomData[0].geomSize * 0.5;
        float halfSizeHoriz = halfSize * ubo.emitter.sizeScale;
        vec4 position = gl_in[0].gl_Position;
        float lifePercent = geomData[0].geomLife / geomData[0].geomStartLife;

        gl_Position = ubo.projection * (gl_in[0].gl_Position + ubo.view * ubo.model * vec4(rotateVector(-halfSize, halfSizeHoriz), 0.0, 0.0 ));
        fragTexCoord = vec2(0.0, 1.0);
        fragColor = geomData[0].geomColor;
        fragLifePercent = lifePercent;
        EmitVertex();

        gl_Position = ubo.projection * (gl_in[0].gl_Position + ubo.view * ubo.model * vec4(rotateVector(-halfSize, -halfSizeHoriz), 0.0, 0.0 ));
        fragTexCoord = vec2(0.0, 0.0);
        fragColor = geomData[0].geomColor;
        fragLifePercent = lifePercent;
        EmitVertex();

        gl_Position = ubo.projection * (gl_in[0].gl_Position + ubo.view * ubo.model * vec4(rotateVector(halfSize, halfSizeHoriz), 0.0, 0.0 ));
        fragTexCoord = vec2(1.0, 1.0);
        fragColor = geomData[0].geomColor;
        fragLifePercent = lifePercent;
        EmitVertex();

        gl_Position = ubo.projection * (gl_in[0].gl_Position + ubo.view * ubo.model * vec4(rotateVector(halfSize, -halfSizeHoriz), 0.0, 0.0 ));
        fragTexCoord = vec2(1.0, 0.0);
        fragColor = geomData[0].geomColor;
        fragLifePercent = lifePercent;
        EmitVertex();

        EndPrimitive();
    }
}
