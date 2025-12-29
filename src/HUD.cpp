#include "HUD.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>

void HUD::initialize() {
    hudShader = std::make_unique<Shader>("shaders/hud.vert", "shaders/hud.frag");
    recreateBuffer();
}

void HUD::recreateBuffer() {
    // create or update VBO with current halfSize
    float s = halfSize;
    float verts[] = {
        -s, 0.0f,
        0.0, s,
        0.0, s,
        s, 0.0f,
        s, 0.0f,
        0.0f, -s,
        0.0f, -s,
        -s, 0.0f
    };

    if (hudVAO == 0) glGenVertexArrays(1, &hudVAO);
    if (hudVBO == 0) glGenBuffers(1, &hudVBO);

    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void HUD::render() {
    if (!hudShader) return;

    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthEnabled) glDisable(GL_DEPTH_TEST);

    hudShader->use();
    hudShader->setVec3("color", color.x, color.y, color.z);

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    float width = (float)vp[2];
    float height = (float)vp[3];
    float aspect = (width != 0.0f && height != 0.0f) ? (width / height) : 1.0f;
    float xScale = 1.0f / aspect;
    hudShader->setVec2("uScale", xScale, 1.0f);

    glBindVertexArray(hudVAO);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);

    if (depthEnabled) glEnable(GL_DEPTH_TEST);
}

void HUD::setCrosshairSize(float halfLength) {
    halfSize = halfLength;
    recreateBuffer();
}

void HUD::setColor(const glm::vec3 &c) {
    color = c;
}

HUD::~HUD() {
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
}
