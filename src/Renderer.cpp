#include "Renderer.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(int width, int height) : width(width), height(height) {}

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    shader = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    portalShader = std::make_unique<Shader>("shaders/screen.vert", "shaders/screen.frag");
    // HUD manager
    hud = std::make_unique<HUD>();
    hud->initialize();
}

void Renderer::resize(int width, int height) {
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
}

void Renderer::drawScene(Scene &scene, Shader &shader, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", viewPos);

    // Directional light
    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // Point light
    shader.setVec3("pointLights[0].position", scene.lightPos);
    shader.setVec3("pointLights[0].ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);

    shader.setFloat("material.shininess", 32.0f);

    for (auto &obj : scene.objects) {
        obj->draw(shader);
    }

    if (scene.skybox) {
        scene.skybox->draw(view, projection);
    }
}

void Renderer::renderPortal(Scene &scene, Portal *portal, Camera &camera, const glm::mat4 &projection) {
    glm::mat4 portalView = portal->beginRender(camera);

    // --- Oblique Frustum Clipping ---
    glm::vec4 worldPlane = portal->getPlaneEquation();
    glm::vec4 viewPlane = worldPlane * glm::inverse(portalView);

    glm::mat4 obliqueProjection = projection;
    glm::vec4 q = glm::inverse(obliqueProjection) * glm::vec4(
        (viewPlane.x > 0.0f ? 1.0f : -1.0f),
        (viewPlane.y > 0.0f ? 1.0f : -1.0f),
        1.0f,
        1.0f
    );
    glm::vec4 c = viewPlane * (2.0f / glm::dot(viewPlane, q));

    obliqueProjection[0][2] = c.x - obliqueProjection[0][3];
    obliqueProjection[1][2] = c.y - obliqueProjection[1][3];
    obliqueProjection[2][2] = c.z - obliqueProjection[2][3];
    obliqueProjection[3][2] = c.w - obliqueProjection[3][3];

    glm::vec3 virtualCamPos = glm::vec3(glm::inverse(portalView)[3]);
    drawScene(scene, *shader, portalView, obliqueProjection, virtualCamPos);

    portal->endRender(width, height);
}

void Renderer::render(Scene &scene, Camera &camera) {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // 1. Render Portal Views
    if (scene.portalA) renderPortal(scene, scene.portalA.get(), camera, projection);
    if (scene.portalB) renderPortal(scene, scene.portalB.get(), camera, projection);

    // 2. Render Main Pass
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene(scene, *shader, view, projection, camera.Position);

    // 3. Draw Portals
    portalShader->use();
    portalShader->setMat4("projection", projection);
    portalShader->setMat4("view", view);

    if (scene.portalA) scene.portalA->draw(*portalShader);
    if (scene.portalB) scene.portalB->draw(*portalShader);

    // 4. Draw Portal Gun
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    if (scene.portalGun) scene.portalGun->draw(*shader);

    // Draw HUD
    if (hud) hud->render();
}
