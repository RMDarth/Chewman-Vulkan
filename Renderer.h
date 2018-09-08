#pragma once

class Renderer
{
public:
    Renderer();
    virtual ~Renderer() = default;

    virtual void resizeWindow() = 0;
    virtual void drawFrame() = 0;
    virtual void finishRendering() = 0;
};

