#version 450
#extension GL_ARB_separate_shader_objects : enable
precision highp float;
#include "text.glsl"

layout (std140, set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 viewProjection;
    TextInfo textInfo;
    GlyphInfo font[300];
    TextSymbolInfo textSymbolInfo[100];
} ubo;

layout(location = 0) out vec2 fragTexCoord;

vec2 positions[6] = vec2[](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
);

vec2 texCoord[6] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

float getSymbolUpperBound(uint index)
{
    return ubo.textSymbolInfo[index].y;
}

void main() {
    uint last = ubo.textInfo.symbolCount - 1;

    float left = ubo.textSymbolInfo[0].x - ubo.font[ubo.textSymbolInfo[0].symbolInfoIndex].originX * ubo.textInfo.scale;
    float right = ubo.textSymbolInfo[last].x + (ubo.font[ubo.textSymbolInfo[last].symbolInfoIndex].width - ubo.font[ubo.textSymbolInfo[last].symbolInfoIndex].originX) * ubo.textInfo.scale;

    float up = getSymbolUpperBound(0);
    float down = getSymbolUpperBound(0) + ubo.textInfo.maxGlyphHeight * ubo.textInfo.scale;

    // ubo.textInfo.imageSize
    float leftT = left / ubo.textInfo.imageSize.x;
    float leftP = (leftT) * 2 - 1;
    float rightT = right / ubo.textInfo.imageSize.x;
    float rightP = (rightT) * 2 - 1;
    float upT = up / ubo.textInfo.imageSize.y;
    float upP = (upT) * 2 - 1;
    float downT = down / ubo.textInfo.imageSize.y;
    float downP = (downT) * 2 - 1;

    vec2 pos = positions[gl_VertexIndex];
    pos.x = pos.x < 0 ? leftP : rightP;
    pos.y = pos.y < 0 ? upP : downP;

    vec2 tex = texCoord[gl_VertexIndex];
    tex.x = tex.x < 0.5 ? leftT : rightT;
    tex.y = tex.y < 0.5 ? upT : downT;

    gl_Position = vec4(pos, 0.0, 1.0);
    fragTexCoord = tex;
}