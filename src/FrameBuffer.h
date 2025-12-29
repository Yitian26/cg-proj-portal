#pragma once

#include <iostream>

#include <glad/gl.h>

class FrameBuffer {
public:
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void Bind();
    void Unbind();
    unsigned int GetTextureID() const;
    void Rescale(int width, int height);

private:
    unsigned int fbo;
    unsigned int textureColorBuffer;
    unsigned int rbo;
    int width;
    int height;
};
