#pragma once

#include <memory>
#include "Shader.h"
#include "Scene.h"
#include "Camera.h"
#include "HUD.h"
#include <glad/gl.h>
#include <glm/glm.hpp>

constexpr int MAX_PORTAL_RECURSION = 3;

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer() = default;

    void initialize();
    void render(Scene &scene, Camera &camera);
    void resize(int width, int height);

private:
    void drawScene(Scene &scene, Shader &shader, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos);
    void renderPortal(Scene &scene, Portal *portal, glm::mat4 view, const glm::mat4 &projection, int recursionDepth);
    int width, height;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> portalShader;
    std::unique_ptr<HUD> hud;
};
