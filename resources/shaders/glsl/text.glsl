// Copyright (c) 2018-2019, Igor Barinov

struct TextSymbolInfo
{
    uint symbolInfoIndex;
    uint x;
    uint y;

    uint padding;
};

struct GlyphInfo
{
    uint x;
    uint y;
    uint width;
    uint height;
    int originX;
    int originY;
    uint advance;

    uint _padding;
};

struct TextInfo
{
    ivec2 fontImageSize;
    vec2 imageSize;
    vec4 color;
    uint symbolCount;
    uint maxHeight;
    float scale;
};