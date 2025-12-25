#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glad/gl.h>
#include "Shader.h"

// Simple HUD manager: crosshair + text stubs
class HUD {
public:
    HUD() = default;
    ~HUD();

    // Initialize resources (shaders, buffers)
    void initialize();

    // Render HUD (crosshair + optional text)
    void render();

    // Appearance
    void setCrosshairSize(float halfLength);
    void setColor(const glm::vec3 &c);

    // Text API (stub for future implementation)
    void drawText(const std::string &text, float x, float y, float scale, const glm::vec3 &color) {
        // placeholder: future text rendering implementation
    }

private:
    std::unique_ptr<Shader> hudShader;
    unsigned int hudVAO = 0;
    unsigned int hudVBO = 0;
    float halfSize = 0.02f;
    glm::vec3 color = glm::vec3(0.5f, 1.0f, 1.0f);

    void recreateBuffer();
};
