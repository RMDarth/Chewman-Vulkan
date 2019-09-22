#version 450
#extension GL_ARB_separate_shader_objects : enable
precision highp float;
#include "text.glsl"

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(std140, set = 1, binding = 1) uniform UBO
{
    TextInfo textInfo;
    GlyphInfo font[300];
    TextSymbolInfo textSymbolInfo[100];
} ubo;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec4 getTextColor()
{
    vec2 currentPos = fragTexCoord * ubo.textInfo.imageSize;

    float symbolPos = 0;
    int currentSymbol = -1;
    for (int c = 0; c < ubo.textInfo.symbolCount; c++)
    {
        symbolPos = ubo.textSymbolInfo[c].x;

        if (currentPos.x < symbolPos - ubo.font[ubo.textSymbolInfo[c].symbolInfoIndex].originX * ubo.textInfo.scale)
        {
            currentSymbol = c - 1;
            break;
        }
        currentSymbol = c;
    }

    if (currentSymbol < 0)
        return vec4(0);

    TextSymbolInfo symbolInfo = ubo.textSymbolInfo[currentSymbol];
    GlyphInfo glyph = ubo.font[symbolInfo.symbolInfoIndex];

    if (currentPos.x > symbolInfo.x + (ubo.font[symbolInfo.symbolInfoIndex].width - glyph.originX) * ubo.textInfo.scale)
        return vec4(0);
    float symbolY = symbolInfo.y + (ubo.textInfo.maxHeight - glyph.originY) * ubo.textInfo.scale;
    if (currentPos.y < symbolY)
        return vec4(0);
    if (currentPos.y > symbolY + glyph.height * ubo.textInfo.scale)
        return vec4(0);

    vec2 ub = vec2(glyph.x, glyph.y) / ubo.textInfo.fontImageSize;
    ub += vec2(currentPos.x - symbolInfo.x + glyph.originX * ubo.textInfo.scale, currentPos.y - symbolY) / ubo.textInfo.scale / ubo.textInfo.fontImageSize ;
    return texture(texSampler, ub).rgba;
}

void main()
{
    outColor = getTextColor();
}